/*  leon3_console.h - Console Driver Interface for leon3
 *  
 *  - vga, ps2, uart
 *
 * $Id$
 */

#ifndef __LEON3_CONSOLE_H__
#define __LEON3_CONSOLE_H__

#include <grlib.h>

struct apbuart_priv {
  struct apbuart_regs *regs;
  unsigned int freq_hz;
#if CONSOLE_USE_INTERRUPTS
  int irq;
  void *cookie;
  volatile int sending;
  char *buf;
#endif
}; /* uart private data section */

struct apbvga_priv {
  struct apbvga_regs *regs; /* data, bgcolor, fgcolr */
  unsigned int bitMapBaseAddr;
  unsigned char column;
  unsigned char row;
  unsigned int bgColor;
  unsigned int fgColor;
  unsigned int nLines;
}; /* vga private data section */




#endif

