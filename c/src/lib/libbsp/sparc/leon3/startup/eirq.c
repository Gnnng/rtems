/*
 *  GRLIB/LEON3 extended interrupt controller
 *
 *  COPYRIGHT (c) 2008.
 *  Gaisler Research
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 */

#include <bsp.h>

extern int LEON3_Cpu_Index;

/* GRLIB extended IRQ controller IRQ number */
int LEON3_IrqCtrl_EIrq = -1;

/* Initialize Exteneded Interrupt controller */
void leon3_ext_irq_init(void)
{
  if ( (LEON3_IrqCtrl_Regs->mpstat >> 16) & 0xf ) {
    /* Extended IRQ controller available */
    LEON3_IrqCtrl_EIrq = (LEON3_IrqCtrl_Regs->mpstat >> 16) & 0xf;
  }
}
