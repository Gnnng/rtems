/*  GR-TMTC-1553 PCI Target driver.
 * 
 *  COPYRIGHT (c) 2008.
 *  Aeroflex Gaisler AB.
 *
 *  Configures the GR-TMTC-1553 interface PCI board.
 *  This driver provides a AMBA PnP bus by using the general part
 *  of the AMBA PnP bus driver (ambapp_bus.c).
 *
 *  Driver resources for the AMBA PnP bus provided can be set using
 *  gr_tmtc_1553_set_resources().
 *
 *  The license and distribution terms for this file may be
 *  found in found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  2010-06-21, Kristoffer Glembo
 *   Created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <bsp.h>
#include <rtems/bspIo.h>
#include <pci.h>
#include <pci/access.h>

#include <ambapp.h>
#include <grlib.h>
#include <drvmgr/drvmgr.h>
#include <drvmgr/ambapp_bus.h>
#include <drvmgr/pci_bus.h>
#include <genirq.h>

#include <gr_tmtc_1553.h>


/*#define DEBUG 1 */

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...) 
#endif

/* PCI ID */
#define PCIID_VENDOR_GAISLER		0x1AC8

int gr_tmtc_1553_init1(struct rtems_drvmgr_dev_info *dev);
int gr_tmtc_1553_init2(struct rtems_drvmgr_dev_info *dev);

struct gr_tmtc_1553_ver {
	const unsigned int	amba_freq_hz;	/* The frequency */
	const unsigned int	amba_ioarea;	/* The address where the PnP IOAREA starts at */
};

/* Private data structure for driver */
struct gr_tmtc_1553_priv {
	/* Driver management */
	struct rtems_drvmgr_dev_info	*dev;
	char				prefix[32];

	/* PCI */
	pci_dev_t			pcidev;
	struct pci_dev_info		*devinfo;

	/* IRQ */
	genirq_t			genirq;

	struct gr_tmtc_1553_ver         *version;
	LEON3_IrqCtrl_Regs_Map		*irq;
	struct rtems_drvmgr_mmap_entry	bus_maps[4];

	struct ambapp_bus		abus;
	struct ambapp_mmap		amba_maps[4];
        struct ambapp_config		config;
};

struct gr_tmtc_1553_ver gr_tmtc_1553_ver0 = {
	.amba_freq_hz		= 33333333,
	.amba_ioarea		= 0xfff00000,
};


int ambapp_tmtc_1553_int_register(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr handler,
	void *arg);
int ambapp_tmtc_1553_int_unregister(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr handler,
	void *arg);
int ambapp_tmtc_1553_int_enable(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg);
int ambapp_tmtc_1553_int_disable(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg);
int ambapp_tmtc_1553_int_clear(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg);
int ambapp_tmtc_1553_get_params(
	struct rtems_drvmgr_dev_info *dev,
	struct rtems_drvmgr_bus_params *params);

struct ambapp_ops ambapp_tmtc_1553_ops = {
	.int_register = ambapp_tmtc_1553_int_register,
	.int_unregister = ambapp_tmtc_1553_int_unregister,
	.int_enable = ambapp_tmtc_1553_int_enable,
	.int_disable = ambapp_tmtc_1553_int_disable,
	.int_clear = ambapp_tmtc_1553_int_clear,
	.get_params = ambapp_tmtc_1553_get_params
};

struct rtems_drvmgr_drv_ops gr_tmtc_1553_ops = 
{
	{gr_tmtc_1553_init1, gr_tmtc_1553_init2, NULL, NULL},
	NULL,
	NULL
};

struct pci_dev_id_match gr_tmtc_1553_ids[] = 
{
	PCIID_DEVVEND(PCIID_VENDOR_GAISLER, PCIID_DEVICE_GR_TMTC_1553),
	PCIID_END_TABLE /* Mark end of table */
};

struct pci_drv_info gr_tmtc_1553_info =
{
	{
		DRVMGR_OBJ_DRV,			/* Driver */
		NULL,				/* Next driver */
		NULL,				/* Device list */
		DRIVER_PCI_GAISLER_TMTC_1553_ID, /* Driver ID */
		"GR-TMTC-1553_DRV",		/* Driver Name */
		DRVMGR_BUS_TYPE_PCI,		/* Bus Type */
		&gr_tmtc_1553_ops,
		0,				/* No devices yet */
	},
	&gr_tmtc_1553_ids[0]
};

/* Driver resources configuration for the AMBA bus on the GR-RASTA-IO board.
 * It is declared weak so that the user may override it from the project file,
 * if the default settings are not enough.
 *
 * The configuration consists of an array of configuration pointers, each
 * pointer determine the configuration of one GR-RASTA-IO board. Pointer
 * zero is for board0, pointer 1 for board1 and so on.
 *
 * The array must end with a NULL pointer.
 */
struct rtems_drvmgr_bus_res *gr_tmtc_1553_resources[] __attribute__((weak)) =
{
	NULL
};
int gr_tmtc_1553_resources_cnt = 0;

void gr_tmtc_1553_register_drv(void)
{
	DBG("Registering GR-TMTC-1553 PCI driver\n");
	rtems_drvmgr_drv_register(&gr_tmtc_1553_info.general);
}

void gr_tmtc_1553_isr (int irqno, void *arg)
{
	struct gr_tmtc_1553_priv *priv = arg;
	unsigned int status, tmp;
	int irq;
	tmp = status = priv->irq->ipend;

	/* DBG("GR-RASTA-IO: IRQ 0x%x\n",status); */

	for(irq=0; irq<16; irq++) {
		if ( status & (1<<irq) ) {
			genirq_doirq(priv->genirq, irq);
			priv->irq->iclear = (1<<irq);
			status &= ~(1<<irq);
			if ( status == 0 )
				break;
		}
	}

	/* ACK interrupt, this is because PCI is Level, so the IRQ Controller still drives the IRQ. */
	if ( tmp ) 
		rtems_drvmgr_interrupt_clear(priv->dev, 0, gr_tmtc_1553_isr, (void *)priv);

	DBG("GR-TMTC-1553-IRQ: 0x%x\n", tmp);
}

/* AMBA PP find routines */
int gr_tmtc_1553_dev_find(struct ambapp_dev *dev, int index, int maxdepth, void *arg)
{
	/* Found IRQ/GRPCI controller, stop */
	*(struct ambapp_dev **)arg = dev;
	return 1;
}

int gr_tmtc_1553_hw_init(struct gr_tmtc_1553_priv *priv)
{
	unsigned int data;
	unsigned int *page0 = NULL;
	struct ambapp_dev *tmp;
	int status;
	struct ambapp_ahb_info *ahb;
	unsigned int pci_freq_hz;
	pci_dev_t pcidev = priv->pcidev;
	struct pci_dev_info *devinfo = priv->devinfo;
	uint32_t bar0, bar0_size;

	/* Select version of GR-TMTC-1553 board */
	switch (devinfo->rev) {
		case 0:
			priv->version = &gr_tmtc_1553_ver0;
			break;
		default:
			return -2;
	}

	bar0 = devinfo->resources[0].address;
	bar0_size = devinfo->resources[0].size;
	page0 = (unsigned int *)(bar0 + bar0_size/2); 

	/* Point PAGE0 to start of board address map. RAM at 0xff000000, APB at 0xffc00000, IOAREA at 0xfff000000 */
	/* XXX We assume little endian host with byte twisting enabled here */
	*page0 = 0x010000ff;	/* Set little endian mode on peripheral. */

	/* Scan AMBA Plug&Play */

	/* AMBA MAP bar0 (in CPU) ==> 0x80000000(remote amba address) */
	priv->amba_maps[0].size = 0x1000000;
	priv->amba_maps[0].local_adr = bar0;
	priv->amba_maps[0].remote_adr = 0xff000000;
	
	/* Addresses not matching with map be untouched */
	priv->amba_maps[2].size = 0xfffffff0;
	priv->amba_maps[2].local_adr = 0;
	priv->amba_maps[2].remote_adr = 0;

	/* Mark end of table */
	priv->amba_maps[3].size=0;
	priv->amba_maps[3].local_adr = 0;
	priv->amba_maps[3].remote_adr = 0;

	/* Start AMBA PnP scan at first AHB bus */
	ambapp_scan(&priv->abus,
		bar0 + (priv->version->amba_ioarea & ~0xff000000),
		NULL, &priv->amba_maps[0]);

	/* Frequency is the hsame as the PCI bus frequency */
	rtems_drvmgr_freq_get(priv->dev, NULL, &pci_freq_hz);

	ambapp_freq_init(&priv->abus, NULL, pci_freq_hz);

	/* Find IRQ controller */
	tmp = NULL;
	status = ambapp_for_each(priv->abus.root, (OPTIONS_ALL|OPTIONS_APB_SLVS), VENDOR_GAISLER, GAISLER_IRQMP, 10, gr_tmtc_1553_dev_find, &tmp);
	if ( (status != 1) || !tmp ) {
		return -4;
	}
	priv->irq = (LEON3_IrqCtrl_Regs_Map *)(((struct ambapp_apb_info *)tmp->devinfo)->start);
	/* Set up irq controller */
	priv->irq->mask[0] = 0;
	priv->irq->iclear = 0xffff;
	priv->irq->ilevel = 0;

	priv->bus_maps[0].map_size = priv->amba_maps[0].size;
	priv->bus_maps[0].local_adr = priv->amba_maps[0].local_adr;
	priv->bus_maps[0].remote_adr = priv->amba_maps[0].remote_adr;


	priv->bus_maps[2].map_size = 0xfffffff0; /* AMBA->PCI Window on GR-TMTC-1553 board */
	priv->bus_maps[2].local_adr = 0;
	priv->bus_maps[2].remote_adr = 0;

	priv->bus_maps[3].map_size = 0;
	priv->bus_maps[3].local_adr = 0;
	priv->bus_maps[3].remote_adr = 0;

	/* Successfully registered the board */
	return 0;
}


/* Called when a PCI target is found with the PCI device and vendor ID 
 * given in gr_tmtc_1553_ids[].
 */
int gr_tmtc_1553_init1(struct rtems_drvmgr_dev_info *dev)
{
	struct gr_tmtc_1553_priv *priv;
	struct pci_dev_info *devinfo;
	int status;
	uint32_t bar0, bar0_size;

	/* PCI device does not have the IRQ line register, when PCI autoconf configures it the configuration
	 * is forgotten. We take the IRQ number from the PCI Host device (AMBA device), this works as long
	 * as PCI-IRQs are ored together on the bus.
	 *
	 * Note that this only works on LEON.
	 */
	((struct pci_dev_info *)dev->businfo)->irq = ((struct amba_dev_info *)dev->parent->dev->businfo)->info.irq; 
	
	priv = malloc(sizeof(struct gr_tmtc_1553_priv));
	if ( !priv )
		return DRVMGR_NOMEM;

	memset(priv, 0, sizeof(*priv));
	dev->priv = priv;
	priv->dev = dev;

	/* Determine number of configurations */
	if ( gr_tmtc_1553_resources_cnt == 0 ) {
		while ( gr_tmtc_1553_resources[gr_tmtc_1553_resources_cnt] )
			gr_tmtc_1553_resources_cnt++;
	}

	/* Generate Device prefix */

	strcpy(priv->prefix, "/dev/tmtc1553_0/");
	priv->prefix[19] += dev->minor_drv;
	mkdir(priv->prefix, S_IRWXU | S_IRWXG | S_IRWXO);
	priv->prefix[20] = '/';
	priv->prefix[21] = '\0';

	priv->devinfo = devinfo = (struct pci_dev_info *)dev->businfo;
	priv->pcidev = devinfo->pcidev;
	bar0 = devinfo->resources[0].address;
	bar0_size = devinfo->resources[0].size;
	printf("\n\n--- GR-TMTC-1553[%d] ---\n", dev->minor_drv);
	printf(" PCI BUS: 0x%x, SLOT: 0x%x, FUNCTION: 0x%x\n",
		PCI_DEV_EXPAND(priv->pcidev));
	printf(" PCI VENDOR: 0x%04x, DEVICE: 0x%04x\n",
		devinfo->id.vendor, devinfo->id.device);
	printf(" PCI BAR[0]: 0x%lx - 0x%lx\n", bar0, bar0 + bar0_size - 1);
	printf(" IRQ: %d\n\n\n", devinfo->irq);

	/* all neccessary space assigned to GR-TMTC-1553 target? */
	if (bar0_size == 0)
		return DRVMGR_ENORES;

	priv->genirq = genirq_init(16);
	if ( priv->genirq == NULL ) {
		free(priv);
		dev->priv = NULL;
		return DRVMGR_FAIL;
	}

	if ( status = gr_tmtc_1553_hw_init(priv) ) {
		genirq_destroy(priv->genirq);
		free(priv);
		dev->priv = NULL;
		printf(" Failed to initialize GR-TMTC-1553 HW: %d\n", status);
		return DRVMGR_FAIL;
	}

	/* Init amba bus */
	priv->config.abus = &priv->abus;
	priv->config.ops = &ambapp_tmtc_1553_ops;
	priv->config.mmaps = &priv->bus_maps[0];
	if ( priv->dev->minor_drv < gr_tmtc_1553_resources_cnt ) {
		priv->config.resources = gr_tmtc_1553_resources[priv->dev->minor_drv];
	} else {
		priv->config.resources = NULL;
	}

	/* Create And Register AMBA PnP Bus */
	return ambapp_bus_register(dev, &priv->config);
}

int gr_tmtc_1553_init2(struct rtems_drvmgr_dev_info *dev)
{
	struct gr_tmtc_1553_priv *priv = dev->priv;

	/* install interrupt vector */
	rtems_drvmgr_interrupt_register(priv->dev, 0, gr_tmtc_1553_isr, (void *)priv);

	/* Clear any old interrupt requests */
	rtems_drvmgr_interrupt_clear(priv->dev, 0, gr_tmtc_1553_isr, (void *)priv);

	/* Enable System IRQ so that GR-TMTC-1553 PCI target interrupt goes through.
	 *
	 * It is important to enable it in stage init2. If interrupts were enabled in init1
	 * this might hang the system when more than one PCI target is connected, this is
	 * because PCI interrupts might be shared and PCI target 2 have not initialized and
	 * might therefore drive interrupt already when entering init1().
	 */
	rtems_drvmgr_interrupt_enable(dev, 0, gr_tmtc_1553_isr, (void *)priv);

	return DRVMGR_OK;
}

int ambapp_tmtc_1553_int_register(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr handler,
	void *arg)
{
	struct gr_tmtc_1553_priv *priv = dev->parent->dev->priv;
	rtems_interrupt_level level;
	int status;

	rtems_interrupt_disable(level);

	status = genirq_register(priv->genirq, irq, handler, arg);
	if ( status == 0 ) {
		/* Disable and clear IRQ for first registered handler */
		priv->irq->iclear = (1<<irq);
		priv->irq->mask[0] &= ~(1<<irq); /* mask interrupt source */
	} else if ( status == 1 )
		status = 0;

	rtems_interrupt_enable(level);

	return status;
}

int ambapp_tmtc_1553_int_unregister(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct gr_tmtc_1553_priv *priv = dev->parent->dev->priv;
	rtems_interrupt_level level;
	int status;

	rtems_interrupt_disable(level);

	status = genirq_unregister(priv->genirq, irq, isr, arg);
	if ( status != 0 )
		status = -1;

	rtems_interrupt_enable(level);

	return status;
}

int ambapp_tmtc_1553_int_enable(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct gr_tmtc_1553_priv *priv = dev->parent->dev->priv;
	rtems_interrupt_level level;
	int status;

	DBG("TMTC-1553 IRQ %d: enable\n", irq);

	rtems_interrupt_disable(level);

	status = genirq_enable(priv->genirq, irq, isr, arg);
	if ( status == 0 ) {
		/* Enable IRQ for first enabled handler only */
		priv->irq->mask[0] |= (1<<irq); /* unmask interrupt source */
	} else if ( status == 1 )
		status = 0;

	rtems_interrupt_enable(level);

	return status;
}

int ambapp_tmtc_1553_int_disable(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct gr_tmtc_1553_priv *priv = dev->parent->dev->priv;
	rtems_interrupt_level level;
	int status;

	DBG("TMTC-1553 IRQ %d: disable\n", irq);

	rtems_interrupt_disable(level);

	status = genirq_disable(priv->genirq, irq, isr, arg);
	if ( status == 0 ) {
		/* Disable IRQ only when no enabled handler exists */
		priv->irq->mask[0] &= ~(1<<irq); /* mask interrupt source */
	} else if ( status == 1 )
		status = 0;

	rtems_interrupt_enable(level);

	return status;
}

int ambapp_tmtc_1553_int_clear(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct gr_tmtc_1553_priv *priv = dev->parent->dev->priv;

	if ( genirq_check(priv->genirq, irq) )
		return -1;

	priv->irq->iclear = (1<<irq);

	return 0;
}

int ambapp_tmtc_1553_get_params(struct rtems_drvmgr_dev_info *dev, struct rtems_drvmgr_bus_params *params)
{
	struct gr_tmtc_1553_priv *priv = dev->parent->dev->priv;

	/* Device name prefix pointer, skip /dev */
	params->dev_prefix = &priv->prefix[5];

	return 0;
}

void gr_tmtc_1553_print_dev(struct rtems_drvmgr_dev_info *dev, int options)
{
	struct gr_tmtc_1553_priv *priv = dev->priv;
	struct pci_dev_info *devinfo = priv->devinfo;
	int i;
	uint32_t bar0, bar0_size;

	/* Print */
	printf("--- GR-TMTC-1553 [bus 0x%x, dev 0x%x, fun 0x%x] ---\n",
		PCI_DEV_EXPAND(priv->pcidev));

	bar0 = devinfo->resources[0].address;
	bar0_size = devinfo->resources[0].size;

	printf(" PCI BAR[0]: 0x%lx - 0x%lx\n", bar0, bar0 + bar0_size - 1);
	printf(" IRQ REGS:        0x%x\n", (unsigned int)priv->irq);
	printf(" IRQ:             %d\n", devinfo->irq);
	printf(" FREQ:            %d Hz\n", priv->version->amba_freq_hz);
	printf(" IMASK:           0x%08x\n", priv->irq->mask[0]);
	printf(" IPEND:           0x%08x\n", priv->irq->ipend);

	/* Print amba config */
	if ( options & TMTC_1553_OPTIONS_AMBA ) {
		ambapp_print(&priv->abus, 10);
	}
#if 0
	/* Print IRQ handlers and their arguments */
	if ( options & TMTC_1553_OPTIONS_IRQ ) {
		for(i=0; i<16; i++) {
			printf(" IRQ[%02d]:         0x%x, arg: 0x%x\n", 
				i, (unsigned int)priv->isrs[i].handler, (unsigned int)priv->isrs[i].arg);
		}
	}
#endif
}

void gr_tmtc_1553_print(int options)
{
	struct pci_drv_info *drv = &gr_tmtc_1553_info;
	struct rtems_drvmgr_dev_info *dev;

	dev = drv->general.dev;
	while(dev) {
		gr_tmtc_1553_print_dev(dev, options);
		dev = dev->next_in_drv;
	}
}
