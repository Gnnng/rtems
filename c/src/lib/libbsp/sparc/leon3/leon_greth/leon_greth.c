/*
 *  LEON3 Opencores Ethernet MAC Configuration Information
 *
 *  COPYRIGHT (c) 2004.
 *  Gaisler Research
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  leon_open_eth.c,v 1.1.2.1 2005/10/05 19:26:00 joel Exp
 */


#include <bsp.h>
#include <libchip/greth.h>
/*#if (GRETH_DEBUG & GRETH_DEBUG_PRINT_REGISTERS)*/
#include <stdio.h>
/*#endif*/

/*
 * Default sizes of transmit and receive descriptor areas
 */
#define RDA_COUNT     32
#define TDA_COUNT     32

greth_configuration_t leon_greth_configuration;

int rtems_leon_greth_driver_attach(
  struct rtems_bsdnet_ifconfig *config,
  int attach
)
{
  unsigned int base_addr = 0; /* avoid warnings */
  unsigned int eth_irq = 0;   /* avoid warnings */
  struct ambapp_dev *adev;
  struct ambapp_common_info *apb;

  /* Scan for MAC AHB slave interface */
  adev = ambapp_for_each(&ambapp_plb, (OPTIONS_ALL|OPTIONS_APB_SLVS), 
                                 VENDOR_GAISLER, GAISLER_ETHMAC,
                                 ambapp_find_by_idx, NULL);
  if (adev) {
    apb = (struct ambapp_common_info *)adev->devinfo;
    base_addr = apb->start;
    eth_irq = apb->irq;

    /* clear control register and reset NIC */
    *(volatile int *) base_addr = 0;
    *(volatile int *) base_addr = GRETH_CTRL_RST;
    *(volatile int *) base_addr = 0;
    leon_greth_configuration.base_address = base_addr;
    leon_greth_configuration.vector = eth_irq + 0x10;
    leon_greth_configuration.txd_count = TDA_COUNT;
    leon_greth_configuration.rxd_count = RDA_COUNT;
    if (rtems_greth_driver_attach( config, &leon_greth_configuration )) {
      LEON_Clear_interrupt(eth_irq);
      LEON_Unmask_interrupt(eth_irq);
    }
  }
  return 0;
}
