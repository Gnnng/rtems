/*  SPWTDP - SpaceWire Time Distribution Protocol. The driver provides
 *  device discovery and interrupt management.
 *
 *  COPYRIGHT (c) 2013.
 *  Aeroflex Gaisler AB
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 */

#ifndef __SPWTDP_H__
#define __SPWTDP_H__

#define SPWTDP_IRQ_S		0x001
#define SPWTDP_IRQ_TR		0x002
#define SPWTDP_IRQ_TM		0x004
#define SPWTDP_IRQ_TT		0x008
#define SPWTDP_IRQ_DIR		0x010
#define SPWTDP_IRQ_DIT		0x020
#define SPWTDP_IRQ_EDI0		0x040
#define SPWTDP_IRQ_EDI1		0x080
#define SPWTDP_IRQ_EDI2		0x100
#define SPWTDP_IRQ_EDI3		0x200

/* SPWTDP Register layout */
struct spwtdp_regs {
	volatile unsigned int conf[4];       /* 00 */
	volatile unsigned int stat[4];       /* 10 */
	volatile unsigned int cmd_ctrl;      /* 20 */
	volatile unsigned int cmd_et[5];     /* 24 */
	volatile unsigned int resv1[2];      /* 38 */
	volatile unsigned int dat_ctrl;      /* 40 */
	volatile unsigned int dat_et[5];     /* 44 */
	volatile unsigned int resv2[2];      /* 58 */
	volatile unsigned int ts_rx_ctrl;    /* 60 */
	volatile unsigned int ts_rx_et[5];   /* 64 */
	volatile unsigned int resv3[2];      /* 78 */
	volatile unsigned int ts_tx_ctrl;    /* 80 */
	volatile unsigned int ts_tx_et[5];   /* 84 */
	volatile unsigned int resv4[2];      /* 98 */
	volatile unsigned int lat_ctrl;      /* A0 */
	volatile unsigned int lat_et[5];     /* A4 */
	volatile unsigned int resv5[2];      /* B8 */
	volatile unsigned int ien;           /* C0 */
	volatile unsigned int ists;          /* C4 */
	volatile unsigned int resv6[14];     /* C8..FC */

	/* External Datation */
	volatile unsigned int edmask[4];     /* 100..10C */
	struct {
		volatile unsigned int ctrl;          /* 110 */
		volatile unsigned int et[5];         /* 114..124 */
		volatile unsigned int resv0[2];      /* 128..12C */
	} edat[4];
};

/* SPWTDP Statistics gathered by driver */
struct spwtdp_stats {

	/* IRQ Stats */
	unsigned int nirqs;
	unsigned int irq_tx;
	unsigned int irq_rx;
	unsigned int time_ccsds_tx;
	unsigned int tc_tx;
	unsigned int tc_rx;
	unsigned int sync;
};

/* Function ISR callback prototype
 *
 * ists    - Interrupt STatus register of the SPWTDP core read by ISR
 * data    - Custom data provided by user
 */
typedef void (*spwtdp_isr_t)(unsigned int ists, void *data);

/* Open a SPWTDP device by minor number. A SPWTDP device can only by opened
 * once. The handle returned must be used as the input parameter 'spwtdp' in 
 * the rest of the calls in the function interface.
 */
extern void *spwtdp_open(int minor);

/* Close a previously opened SPWTDP device */
extern void spwtdp_close(void *spwtdp);

/* Reset SPWTDP Core */
extern int spwtdp_reset(void *spwtdp);

/* Enable Interrupts at Interrupt controller */
extern void spwtdp_int_enable(void *spwtdp);

/* Disable Interrupts at Interrupt controller */
extern void spwtdp_int_disable(void *spwtdp);

/* Clear Statistics gathered by the driver */
extern void spwtdp_clr_stats(void *spwtdp);

/* Get Statistics gathered by the driver. The statistics are stored into
 * the location pointed to by 'stats'.
 */
extern void spwtdp_get_stats(void *spwtdp, struct spwtdp_stats *stats);

/* Register an Interrupt handler and custom data, the function call is
 * removed by setting func to NULL.
 *
 * The driver's interrupt handler is installed on open(), however the user
 * callback called from the driver's ISR is installed using this function.
 */
extern void spwtdp_int_register(void *spwtdp, spwtdp_isr_t func, void *data);

/* Clear interrupts */
extern void spwtdp_clear_irqs(void *spwtdp, int irqs);

/* Enable interrupts */
extern void spwtdp_enable_irqs(void *spwtdp, int irqs);

/* Get Register */
extern struct spwtdp_regs *spwtdp_get_regs(void *spwtdp);

/* Register the SPWTDP Driver to the Driver Manager */
extern void spwtdp_register_drv(void);

#endif
