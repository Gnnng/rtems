/*  GRLIB GRPCI PCI HOST driver.
 * 
 *  COPYRIGHT (c) 2008.
 *  Aeroflex Gaisler AB.
 *
 *  Configures the GRPCI core and initialize,
 *   - the PCI Library (pci.c)
 *   - the general part of the PCI Bus driver (pci_bus.c)
 *  
 *  System interrupt assigned to PCI interrupt (INTA#..INTD#) is by
 *  default taken from Plug and Play, but may be overridden by the 
 *  driver resources INTA#..INTD#.
 *
 *  The license and distribution terms for this file may be
 *  found in found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  2008-12-06, Daniel Hellstrom <daniel@gaisler.com>
 *   Created
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtems/bspIo.h>
#include <libcpu/byteorder.h>
#include <libcpu/access.h>
#include <pci.h>
#include <pci/cfg.h>

#include <drvmgr/drvmgr.h>
#include <drvmgr/ambapp_bus.h>
#include <ambapp.h>
#include <drvmgr/pci_bus.h>

#define DMAPCI_ADDR 0x80000500

/* Configuration options */
#define SYSTEM_MAINMEM_START 0x40000000

/* If defined to 1 - byte twisting is enabled by default */
#define DEFAULT_BT_ENABLED 0

/* Interrupt assignment. Set to other value than 0xff in order to 
 * override defaults and plug&play information
 */
#ifndef GRPCI_INTA_SYSIRQ
 #define GRPCI_INTA_SYSIRQ 0xff
#endif
#ifndef GRPCI_INTB_SYSIRQ
 #define GRPCI_INTB_SYSIRQ 0xff
#endif
#ifndef GRPCI_INTC_SYSIRQ
 #define GRPCI_INTC_SYSIRQ 0xff
#endif
#ifndef GRPCI_INTD_SYSIRQ
 #define GRPCI_INTD_SYSIRQ 0xff
#endif

#define PAGE0_BTEN_BIT    0
#define PAGE0_BTEN        (1<<PAGE0_BTEN_BIT)

#define CFGSTAT_HOST_BIT  13
#define CFGSTAT_HOST      (1<<CFGSTAT_HOST_BIT)

/*#define DEBUG 1*/

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...) 
#endif

#define PCI_INVALID_VENDORDEVICEID	0xffffffff
#define PCI_MULTI_FUNCTION		0x80

/*
 * Bit encode for PCI_CONFIG_HEADER_TYPE register
 */
struct grpci_regs {
	volatile unsigned int cfg_stat;
	volatile unsigned int bar0;
	volatile unsigned int page0;
	volatile unsigned int bar1;
	volatile unsigned int page1;
	volatile unsigned int iomap;
	volatile unsigned int stat_cmd;
	volatile unsigned int irq;
};

struct grpci_priv *grpcipriv = NULL;
static int grpci_minor = 0;
static unsigned int *pcidma = (unsigned int *)DMAPCI_ADDR;

/* PCI Interrupt assignment. Connects an PCI interrupt pin (INTA#..INTD#)
 * to a system interrupt number.
 */
unsigned char grpci_pci_irq_table[4] =
{
	/* INTA# */	GRPCI_INTA_SYSIRQ,
	/* INTB# */	GRPCI_INTB_SYSIRQ,
	/* INTC# */	GRPCI_INTC_SYSIRQ,
	/* INTD# */	GRPCI_INTD_SYSIRQ
};

/* Driver private data struture */
struct grpci_priv {
	struct rtems_drvmgr_dev_info	*dev;
	struct grpci_regs		*regs;
	int				irq;
	int				minor;

	int				bt_enabled;
	unsigned int			pci_area;
	unsigned int			pci_area_end;
	unsigned int			pci_io;    
	unsigned int			pci_conf;
	unsigned int			pci_conf_end;

	uint32_t			devVend; /* Host PCI Vendor/Device ID */

	/* PCI Bus layer configuration */
	struct pcibus_config	config;
};

int grpci_init1(struct rtems_drvmgr_dev_info *dev);

/* GRPCI DRIVER */

struct rtems_drvmgr_drv_ops grpci_ops = 
{
	.init = {grpci_init1, NULL, NULL, NULL},
	.remove = NULL,
	.info = NULL
};

struct amba_dev_id grpci_ids[] = 
{
	{VENDOR_GAISLER, GAISLER_PCIFBRG},
	{0, 0}		/* Mark end of table */
};

struct amba_drv_info grpci_info =
{
	{
		DRVMGR_OBJ_DRV,			/* Driver */
		NULL,				/* Next driver */
		NULL,				/* Device list */
		DRIVER_AMBAPP_GAISLER_GRPCI_ID,	/* Driver ID */
		"GRPCI_DRV",			/* Driver Name */
		DRVMGR_BUS_TYPE_AMBAPP,		/* Bus Type */
		&grpci_ops,
		0,				/* No devices yet */
		sizeof(struct grpci_priv),	/* Make drvmgr alloc private */
	},
	&grpci_ids[0]
};

void grpci_register_drv(void)
{
	DBG("Registering GRPCI driver\n");
	rtems_drvmgr_drv_register(&grpci_info.general);
}

int grpci_cfg_r32(pci_dev_t dev, int ofs, uint32_t *val)
{
	struct grpci_priv *priv = grpcipriv;
	volatile uint32_t *pci_conf;
	unsigned int devfn = PCI_DEV_DEVFUNC(dev);
	int retval;
	int bus = PCI_DEV_BUS(dev);

	if (ofs & 3)
		return PCISTS_EINVAL;

	if (PCI_DEV_SLOT(dev) > 21) {
		*val = 0xffffffff;
		return PCISTS_OK;
	}

	/* Select bus */
	priv->regs->cfg_stat = (priv->regs->cfg_stat & ~(0xf<<23)) | (bus<<23);

	pci_conf = (volatile uint32_t *)(priv->pci_conf | (devfn << 8) | ofs);

	if (priv->bt_enabled) {
		*val =  CPU_swap_u32(*pci_conf);
	} else {
		*val = *pci_conf;
	}

	if (priv->regs->cfg_stat & 0x100) {
		*val = 0xffffffff;
		retval = PCISTS_MSTABRT;
	} else
		retval = PCISTS_OK;

	DBG("pci_read: [%x:%x:%x] reg: 0x%x => addr: 0x%x, val: 0x%x\n",
		PCI_DEV_EXPAND(dev), ofs, pci_conf, *val);

	return retval;
}


int grpci_cfg_r16(pci_dev_t dev, int ofs, uint16_t *val)
{
	uint32_t v;
	int retval;

	if (ofs & 1)
		return PCISTS_EINVAL;

	retval = grpci_cfg_r32(dev, ofs & ~0x3, &v);
	*val = 0xffff & (v >> (8*(ofs & 0x3)));

	return retval;
}

int grpci_cfg_r8(pci_dev_t dev, int ofs, uint8_t *val)
{
	uint32_t v;
	int retval;

	retval = grpci_cfg_r32(dev, ofs & ~0x3, &v);

	*val = 0xff & (v >> (8*(ofs & 3)));

	return retval;
}

int grpci_cfg_w32(pci_dev_t dev, int ofs, uint32_t val)
{
	struct grpci_priv *priv = grpcipriv;
	volatile uint32_t *pci_conf;
	uint32_t value, devfn = PCI_DEV_DEVFUNC(dev);
	int bus = PCI_DEV_BUS(dev);

	if (ofs & 0x3)
		return PCISTS_EINVAL;

	if (PCI_DEV_SLOT(dev) > 21)
		return PCISTS_MSTABRT;

	/* Select bus */
	priv->regs->cfg_stat = (priv->regs->cfg_stat & ~(0xf<<23)) | (bus<<23);

	pci_conf = (volatile uint32_t *)(priv->pci_conf | (devfn << 8) | ofs);

	if ( priv->bt_enabled ) {
		value = CPU_swap_u32(val);
	} else {
		value = val;
	}

	*pci_conf = value;

	DBG("pci_write - [%x:%x:%x] reg: 0x%x => addr: 0x%x, val: 0x%x\n",
		PCI_DEV_EXPAND(dev), ofs, pci_conf, value);

	return PCISTS_OK;
}

int grpci_cfg_w16(pci_dev_t dev, int ofs, uint16_t val)
{
	uint32_t v;
	int retval;

	if (ofs & 1)
		return PCISTS_EINVAL;

	retval = grpci_cfg_r32(dev, ofs & ~0x3, &v);
	if (retval != PCISTS_OK)
		return retval;

	v = (v & ~(0xffff << (8*(ofs&3)))) | ((0xffff&val) << (8*(ofs&3)));

	return grpci_cfg_w32(dev, ofs & ~0x3, v);
}

int grpci_cfg_w8(pci_dev_t dev, int ofs, uint8_t val)
{
	uint32_t v;

	grpci_cfg_r32(dev, ofs & ~0x3, &v);

	v = (v & ~(0xff << (8*(ofs&3)))) | ((0xff&val) << (8*(ofs&3)));

	return grpci_cfg_w32(dev, ofs & ~0x3, v);
}

/* Return the assigned system IRQ number that corresponds to the PCI
 * "Interrupt Pin" information from configuration space.
 *
 * The IRQ information is stored in the grpci_pci_irq_table configurable
 * by the user.
 *
 * Returns the "system IRQ" for the PCI INTA#..INTD# pin in irq_pin. Returns
 * 0xff if not assigned.
 */
uint8_t grpci_bus0_irq_map(pci_dev_t dev, int irq_pin)
{
	uint8_t sysIrqNr = 0; /* not assigned */
	int irq_group;

	if ( (irq_pin >= 1) && (irq_pin <= 4) ) {
		/* Use default IRQ decoding on PCI BUS0 according slot numbering */
		irq_group = PCI_DEV_SLOT(dev) & 0x3;
		irq_pin = ((irq_pin - 1) + irq_group) & 0x3;
		/* Valid PCI "Interrupt Pin" number */
		sysIrqNr = grpci_pci_irq_table[irq_pin];
	}
	return sysIrqNr;
}

struct pci_access_drv grpci_access_drv = {
	.cfg =
	{
		grpci_cfg_r8,
		grpci_cfg_r16,
		grpci_cfg_r32,
		grpci_cfg_w8,
		grpci_cfg_w16,
		grpci_cfg_w32,
	},
	.io =
	{
		sparc_ld8,
		sparc_ld_le16,
		sparc_ld_le32,
		sparc_st8,
		sparc_st_le16,
		sparc_st_le32,
	},
	.translate = NULL,
};

struct pci_io_ops grpci_io_ops_be =
{
	sparc_ld8,
	sparc_ld_be16,
	sparc_ld_be32,
	sparc_st8,
	sparc_st_be16,
	sparc_st_be32,
};

int grpci_hw_init(struct grpci_priv *priv)
{
	volatile unsigned int *mbar0, *page0;
	uint32_t data, addr, mbar0size;
	pci_dev_t host = PCI_DEV(0, 0, 0);

	mbar0 = (volatile unsigned int *)priv->pci_area;

	if ( !priv->bt_enabled && ((priv->regs->page0 & PAGE0_BTEN) == PAGE0_BTEN) ) {
		/* Byte twisting is on, turn it off */
		grpci_cfg_w32(host, PCI_BASE_ADDRESS_0, 0xffffffff);
		grpci_cfg_r32(host, PCI_BASE_ADDRESS_0, &addr);
		/* Setup bar0 to nonzero value */
		grpci_cfg_w32(host, PCI_BASE_ADDRESS_0,
				CPU_swap_u32(0x80000000));
		/* page0 is accessed through upper half of bar0 */
		addr = (~CPU_swap_u32(addr)+1)>>1;
		mbar0size = addr*2;
		DBG("GRPCI: Size of MBAR0: 0x%x, MBAR0: 0x%x(lower) 0x%x(upper)\n",mbar0size,((unsigned int)mbar0),((unsigned int)mbar0)+mbar0size/2);
		page0 = &mbar0[mbar0size/8];
		DBG("GRPCI: PAGE0 reg address: 0x%x (0x%x)\n",((unsigned int)mbar0)+mbar0size/2,page0);
		priv->regs->cfg_stat = (priv->regs->cfg_stat & (~0xf0000000)) | 0x80000000;    /* Setup mmap reg so we can reach bar0 */ 
		*page0 = 0<<PAGE0_BTEN_BIT;                                         /* Disable bytetwisting ... */
	}

	/* Get the GRPCI Host PCI ID */
	grpci_cfg_r32(host, PCI_VENDOR_ID, &priv->devVend);

	/* set 1:1 mapping between AHB -> PCI memory */
	priv->regs->cfg_stat = (priv->regs->cfg_stat & 0x0fffffff) | priv->pci_area;
	
	/* and map system RAM at pci address 0x40000000 */ 
	grpci_cfg_w32(host, PCI_BASE_ADDRESS_1, SYSTEM_MAINMEM_START);
	priv->regs->page1 = SYSTEM_MAINMEM_START;

	/* Translate I/O accesses 1:1 */
	priv->regs->iomap = priv->pci_io & 0xffff0000;

	/* set as bus master and enable pci memory responses */  
	grpci_cfg_r32(host, PCI_COMMAND, &data);
	data |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
	grpci_cfg_w32(host, PCI_COMMAND, data);

	/* unmask all PCI interrupts at PCI Core, not all GRPCI cores support
	 * this
	 */
	priv->regs->irq = 0xf0000;

	/* Successful */
	return 0;
}

/* Initializes the GRPCI core and driver, must be called before calling init_pci() 
 *
 * Return values
 *  0             Successful initalization
 *  -1            Error during initialization, for example "PCI core not found".
 *  -2            Error PCI controller not HOST (targets not supported)
 *  -3            Error due to GRPCI hardware initialization
 *  -4            Error registering driver to PCI layer
 */
int grpci_init(struct grpci_priv *priv)
{
	struct ambapp_apb_info *apb;
	struct ambapp_ahb_info *ahb;
	int pin;
	union rtems_drvmgr_key_value *value;
	char keyname[6];
	struct amba_dev_info *ainfo = priv->dev->businfo;

	/* Find PCI core from Plug&Play information */
	apb = ainfo->info.apb_slv;
	ahb = ainfo->info.ahb_slv;

	/* Found PCI core, init private structure */
	priv->irq = apb->irq;
	priv->regs = (struct grpci_regs *)apb->start;
	priv->bt_enabled = DEFAULT_BT_ENABLED;

	/* Calculate the PCI windows 
	 *  AMBA->PCI Window:                       AHB SLAVE AREA0
	 *  AMBA->PCI I/O cycles Window:            AHB SLAVE AREA1 Lower half
	 *  AMBA->PCI Configuration cycles Window:  AHB SLAVE AREA1 Upper half
	 */
	priv->pci_area     = ahb->start[0];
	priv->pci_area_end = ahb->start[0] + ahb->mask[0];
	priv->pci_io       = ahb->start[1];
	priv->pci_conf     = ahb->start[1] + (ahb->mask[1] >> 1);
	priv->pci_conf_end = ahb->start[1] + ahb->mask[1];

	/* On systems where PCI I/O area and configuration area is apart of the "PCI Window" 
	 * the PCI Window stops at the start of the PCI I/O area
	 */
	if ( (priv->pci_io > priv->pci_area) && (priv->pci_io < (priv->pci_area_end-1)) ) {
		priv->pci_area_end = priv->pci_io;
	}

	/* Init PCI interrupt assignment table to all use the interrupt routed through
	 * the GRPCI core.
	 */
	strcpy(keyname, "INTX#");
	for (pin=1; pin<5; pin++) {
		if ( grpci_pci_irq_table[pin-1] == 0xff ) {
			grpci_pci_irq_table[pin-1] = priv->irq;

			/* User may override Both hardcoded IRQ setup and Plug & Play IRQ */
			keyname[3] = 'A' + (pin-1);
			value = rtems_drvmgr_dev_key_get(priv->dev, keyname, KEY_TYPE_INT);
			if ( value )
				grpci_pci_irq_table[pin-1] = value->i;
		}
	}

	/* User may override DEFAULT_BT_ENABLED to enable/disable byte twisting */
	value = rtems_drvmgr_dev_key_get(priv->dev, "byteTwisting", KEY_TYPE_INT);
	if ( value )
		priv->bt_enabled = value->i;

	/* This driver only support HOST systems, we check for HOST */
	if ( !(priv->regs->cfg_stat & CFGSTAT_HOST) ) {
		/* Target not supported */
		return -2;
	}

	/* Init the PCI Core */
	if ( grpci_hw_init(priv) ) {
		return -3;
	}

	return 0;
}

/* Called when a core is found with the AMBA device and vendor ID 
 * given in grpci_ids[]. IRQ, Console does not work here
 */
int grpci_init1(struct rtems_drvmgr_dev_info *dev)
{
	int status;
	struct grpci_priv *priv;
	struct pci_auto_setup grpci_auto_cfg;

	DBG("GRPCI[%d] on bus %s\n", dev->minor_drv, dev->parent->dev->name);

	if ( grpci_minor != 0 ) {
		DBG("Driver only supports one PCI core\n");
		return DRVMGR_FAIL;
	}

	if ( (strcmp(dev->parent->dev->drv->name, "AMBAPP_GRLIB_DRV") != 0) && 
	     (strcmp(dev->parent->dev->drv->name, "AMBAPP_LEON2_DRV") != 0) ) {
		/* We only support GRPCI driver on local bus */
		return DRVMGR_FAIL;
	}

	priv = dev->priv;
	if ( !priv )
		return DRVMGR_NOMEM;

	priv->dev = dev;
	priv->minor = grpci_minor++;

	grpcipriv = priv;
	status = grpci_init(priv);
	if (status) {
		printf("Failed to initialize grpci driver %d\n", status);
		return DRVMGR_FAIL;
	}


	/* Register the PCI core at the PCI layers */

	if (priv->bt_enabled) {
		memcpy(&grpci_access_drv.io, &grpci_io_ops_be,
						sizeof(grpci_io_ops_be));
	}

	if (pci_access_drv_register(&grpci_access_drv)) {
		/* Access routines registration failed */
		return DRVMGR_FAIL;
	}

	/* Prepare memory MAP */
	grpci_auto_cfg.options = 0;
	grpci_auto_cfg.mem_start = 0;
	grpci_auto_cfg.mem_size = 0;
	grpci_auto_cfg.memio_start = priv->pci_area;
	grpci_auto_cfg.memio_size = priv->pci_area_end - priv->pci_area;
	grpci_auto_cfg.io_start = priv->pci_io;
	grpci_auto_cfg.io_size = priv->pci_conf - priv->pci_io;
	grpci_auto_cfg.irq_map = grpci_bus0_irq_map;
	grpci_auto_cfg.irq_route = NULL; /* use standard routing */
	pci_config_register(&grpci_auto_cfg);

	if (pci_config_init()) {
		/* PCI configuration failed */
		return DRVMGR_FAIL;
	}

	return pcibus_register(dev, &priv->config);
}

/* DMA functions which uses GRPCIs optional DMA controller (len in words) */
int grpci_dma_to_pci(unsigned int ahb_addr, unsigned int pci_addr, unsigned int len) {
    int ret = 0;

    pcidma[0] = 0x82;
    pcidma[1] = ahb_addr;
    pcidma[2] = pci_addr;
    pcidma[3] = len;
    pcidma[0] = 0x83;        

    while ( (pcidma[0] & 0x4) == 0)
        ;

    if (pcidma[0] & 0x8) { /* error */ 
        ret = -1;
    }

    pcidma[0] |= 0xC; 
    return ret;

}

int grpci_dma_from_pci(unsigned int ahb_addr, unsigned int pci_addr, unsigned int len) {
    int ret = 0;

    pcidma[0] = 0x80;
    pcidma[1] = ahb_addr;
    pcidma[2] = pci_addr;
    pcidma[3] = len;
    pcidma[0] = 0x81;        

    while ( (pcidma[0] & 0x4) == 0)
        ;

    if (pcidma[0] & 0x8) { /* error */ 
        ret = -1;
    }

    pcidma[0] |= 0xC; 
    return ret;

}