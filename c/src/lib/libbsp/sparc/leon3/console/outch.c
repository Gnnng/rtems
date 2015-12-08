/*
 * outch.c  - This file contains code for displaying characters
 *	      on the console uisng information that should be
 *	      maintained by the BIOS in its data Area.
 *
 * Copyright (C) 1998  Eric Valette (valette@crf.canon.fr)
 *                     Canon Centre Recherche France.
 *
 *  The license and distribution terms for this file may be
 *  found in found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 * Till Straumann <strauman@slac.stanford.edu>, 2003/9:
 *  - added handling of basic escape sequences (cursor movement
 *    and erasing; just enough for the line editor 'libtecla' to
 *    work...)
 *
 * Gong Deli <gnnnnng@gmail.com>, 2015/12
 *  - port to BSP leon3
 *
 * $Id$
 */

#include <bsp.h>

#include <rtems/bspIo.h>
#include <stdlib.h>
#include <string.h>
#include <amba.h>
#include <leon3_console.h>

#define TAB_SPACE 4
#define ioCrtBaseAddr 0x80000600
#define MAX_TEXT_BUFFER_LINES 51
#define TEXT_COLOR (0)

extern struct apbvga_priv apbvga_con;

/* TODO: macCol & maxRow should be read from hardware configuration, but
 * the APBVGA module doesn't contain these information, so we define them
 * here
 */
static unsigned short maxCol = 80;
static unsigned short maxRow = 37;

unsigned int   *bitMapBaseAddr = &apbvga_con.bitMapBaseAddr;
unsigned char  *row            = &apbvga_con.row;
unsigned char  *column         = &apbvga_con.column;
unsigned int   *bgColor        = &apbvga_con.bgColor;
unsigned int   *fgColor        = &apbvga_con.fgColor;
unsigned int   *nLines         = &apbvga_con.nLines;

void wr_cursor(int newPos, unsigned short ioBaseAddr) {
  return;
}

void wDataReg(unsigned int addr, char c) {
  unsigned int x = (((addr) & 0xfff) << 8) | ((c) & 0xff);
  apbvga_con.regs->data = (((addr) & 0xfff) << 8) | ((c) & 0xff);
}

void wbgcReg(char r, char g, char b) {
  apbvga_con.bgColor = ( ((r) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((b) & 0xff);
  apbvga_con.regs->bgcolor = apbvga_con.bgColor;
  printk("Write vga bgcolor reg: %x\n", apbvga_con.bgColor);
}

void wfgcReg(char r, char g, char b) {
  apbvga_con.fgColor = ( ((r) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((b) & 0xff);
  apbvga_con.regs->fgcolor = apbvga_con.fgColor;
  printk("Write vga fgcolor reg: %x\n", apbvga_con.fgColor);
}

static void
scroll(void)
{
    int i, j;				    /* Counters	*/
    unsigned int pt_bitmap;  /* Pointers on the bit-map	*/
    unsigned int newAddr;

    newAddr = *bitMapBaseAddr + maxCol * maxRow;
    *bitMapBaseAddr = (newAddr >= MAX_TEXT_BUFFER_LINES * maxCol) ? 0 : newAddr;
    pt_bitmap = *bitMapBaseAddr;
    /*
     * Directly write new line to trigger the hardware scroll
     * Details about the hardware scroll can be found in grip.pdf
     */
    for (i = 0; i < maxCol; i++) {
	wDataReg(pt_bitmap++, ' ');
    }
}

static void
doCRNL(int cr, int nl)
{
	if (nl) {
    if (++(*row) == maxRow) {
	scroll(); 	/* Scroll the screen now */
	*row = maxRow - 1;
    }
    nLines++;
	}
	if (cr)
    	(*column) = 0;
    /* Move cursor on the next location  */
	if (cr || nl)
    	wr_cursor((*row) * maxCol + (*column), ioCrtBaseAddr);
}

int (*videoHook)(char, int *)=0;

static void
advanceCursor(void)
{
  if (++(*column) == maxCol)
	doCRNL(1,1);
  else
	wr_cursor((*row) * maxCol + (*column), ioCrtBaseAddr);
}

static void
gotorc(int r, int c)
{
	(*column) = c;
	(*row)    = r;

  	wr_cursor((*row) * maxCol + (*column), ioCrtBaseAddr);
}

#define ESC		((char)27)
/* erase current location without moving the cursor */
#define BLANK	((char)0x7f)
static void
videoPutChar(char car)
{
    unsigned int pt_bitmap = *bitMapBaseAddr + (*row) * maxCol + (*column);

    switch (car) {
    case '\b': {
	    if ((*column)) (*column)--;
	    /* Move cursor on the previous location  */
	    wr_cursor((*row) * maxCol + (*column), ioCrtBaseAddr);
	    return;
	}
	case '\t': {
	    int i;

	    i = TAB_SPACE - ((*column) & (TAB_SPACE - 1));
	    (*column) += i;
	    if ((*column) >= maxCol) {
		doCRNL(1,1);
		return;
	    }
	    while (i--)	wDataReg(pt_bitmap++, ' ');
	    wr_cursor((*row) * maxCol + (*column), ioCrtBaseAddr);
	    return;
	}
	case '\n': {
	    doCRNL(0,1);
	    return;
	}
	case 7:		{	/* Bell code must be inserted here */
	    return;
	}
	case '\r' : {
		doCRNL(1,0);
	    return;
	}
	case BLANK: {
	    wDataReg(pt_bitmap, ' ');
		/* DONT move the cursor... */
		return;
	}
   	default: {
	    wDataReg(pt_bitmap, car);
		advanceCursor();
	    return;
	}
    }
}

/* trivial state machine to handle escape sequences:
 *
 *                    ---------------------------------
 *                   |                                 |
 *                   |                                 |
 * KEY:        esc   V    [          DCABHKJ       esc |
 * STATE:   0 -----> 27 -----> '[' ----------> -1 -----
 *          ^\        \          \               \
 * KEY:     | \other   \ other    \ other         \ other
 *           <-------------------------------------
 *
 * in state '-1', the DCABHKJ cases are handled
 *
 * (cursor motion and screen clearing)
 */

#define DONE  (-1)

static int
handleEscape(int oldState, char car)
{
int rval = 0;
int ro,co;

	switch ( oldState ) {
		case DONE:	/*  means the previous char terminated an ESC sequence... */
		case 0:
			if ( 27 == car ) {
				rval = 27;	 /* START of an ESC sequence */
			}
		break;

		case 27:
			if ( '[' == car ) {
				rval = car;  /* received ESC '[', so far */
			} else {
				/* dump suppressed 'ESC'; outch will append the char */
				videoPutChar(ESC);
			}
		break;

		case '[':
			/* handle 'ESC' '[' sequences here */
			ro = (*row); co = (*column);
			rval = DONE; /* done */

			switch (car) {
				case 'D': /* left */
					if ( co > 0 )      co--;
				break;
				case 'C': /* right */
					if ( co < maxCol ) co++;
				break;
				case 'A': /* up    */
					if ( ro > 0 )      ro--;
				break;
				case 'B': /* down */
					if ( ro < maxRow ) ro++;
				break;
				case 'H': /* home */
					ro = co = 0;
				break;
				case 'K': /* clear to end of line */
					while ( (*column) < maxCol - 1 )
                		videoPutChar(' ');
            		videoPutChar(BLANK);
        		break;
        		case 'J': /* clear to end of screen */
					while (  (((*row) < maxRow-1) || ((*column) < maxCol-1)) )
						videoPutChar(' ');
					videoPutChar(BLANK);
        		break;
        		default:
            		videoPutChar(ESC);
            		videoPutChar('[');
					/* DONT move the cursor */
					ro   = -1;
					rval = 0;
        		break;
			}
			/* reset cursor */
			if ( ro >= 0)
				gotorc(ro,co);

		default:
		break;

	}

	return rval;
}

void
clear_screen(void)
{
    int i,j;

    for (j = 0; j < maxRow; j++) {
      for (i = 0; i < maxCol; i++) {
        wDataReg(i + j * maxCol, ' ');
      }
    }
}

/*-------------------------------------------------------------------------+
|         Function: _IBMPC_outch
|      Description: Higher level (console) interface to consPutc.
| Global Variables: None.
|        Arguments: c - character to write to console.
|          Returns: Nothing.
+--------------------------------------------------------------------------*/
void
_IBMPC_outch(char c)
{
static int escaped = 0;

  if ( ! (escaped = handleEscape(escaped, c)) ) {
    if ( '\n' == c )
      videoPutChar('\r');
  	videoPutChar(c);
  }
} /* _IBMPC_outch */

/*-------------------------------------------------------------------------+
|         Function: _IBMPC_initVideo
|      Description: Video system initialization. Hook for any early setup.
| Global Variables: bitMapBaseAddr, ioCrtBaseAddr, maxCol, maxRow, row
|		    column, attribute, nLines;
|        Arguments: None.
|          Returns: Nothing.
+--------------------------------------------------------------------------*/
void
_IBMPC_initVideo(void)
{
    wbgcReg(0, 0, 0);
    wfgcReg(255, 255, 255);
    clear_screen();
    *bitMapBaseAddr = 0;
    (*column)  = 0;
    (*row)     = 0;
    printk("maxCol = %d, maxRow = %d, bgColor = %x, fgColor = %x\n", 
        maxCol, maxRow, apbvga_con.bgColor, apbvga_con.fgColor);
    printk("bitMapBaseAddr = %X\n", *bitMapBaseAddr);
} /* _IBMPC_initVideo */

/* for old DOS compatibility n-curses type of applications */
void gotoxy( int x, int y )
{
  gotorc(y,x);
}

int whereX( void )
{
  return (*row);
}

int whereY( void )
{
  return (*column);
}
