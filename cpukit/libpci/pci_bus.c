/*  PCI bus driver.
 *
 *  COPYRIGHT (c) 2008.
 *  Aeroflex Gaisler.
 *
 *  General part of PCI Bus driver. The driver is typically 
 *  initialized from the PCI host driver separating the host
 *  driver from the common parts in PCI drivers.
 *  The PCI library must be initialized before starting the
 *  PCI bus driver. The PCI library have set up BARs and 
 *  assigned system IRQs for targets.
 *  This PCI bus driver rely on the PCI library (pci.c) for
 *  interrupt registeration (pci_interrupt_register) and PCI 
 *  target set up.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  2008-12-03, Daniel Hellstrom <daniel@gaisler.com>
 *    Created
 *
 */

/* Use PCI Configuration libarary pci_hb RAM device structure to find devices,
 * undefine to access PCI configuration space directly.
 */
#define USE_PCI_CFG_LIB

/* On small systems undefine PCIBUS_INFO to avoid sprintf get dragged in */
#define PCIBUS_INFO

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pci.h>
#ifdef USE_PCI_CFG_LIB
#include <pci/cfg.h>
#endif
#include <pci/irq.h>

#include <drvmgr/drvmgr.h>
#include <drvmgr/pci_bus.h>


#define DBG(args...)
/*#define DBG(args...) printk(args)*/
int pcibus_bus_init1(struct rtems_drvmgr_bus_info *bus);
int pcibus_unite(struct rtems_drvmgr_drv_info *drv, struct rtems_drvmgr_dev_info *dev);
int pcibus_int_register(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg);
int pcibus_int_unregister(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg);
int pcibus_int_enable(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg);
int pcibus_int_disable(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg);
int pcibus_int_clear(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg);
int pcibus_freq_get(
	struct rtems_drvmgr_dev_info *dev,
	int options,
	unsigned int *freq_hz);

int pcibus_get_params(struct rtems_drvmgr_dev_info *dev, struct rtems_drvmgr_bus_params *params);

void pcibus_dev_info(
	struct rtems_drvmgr_dev_info *dev,
	void (*print_line)(void *p, char *str),
	void *p);

struct rtems_drvmgr_bus_ops pcibus_ops =
{
	.init		= 
			{
				pcibus_bus_init1,
				NULL,
				NULL,
				NULL
			},
	.remove		= NULL,
	.unite		= pcibus_unite,
	.int_register	= pcibus_int_register,
	.int_unregister	= pcibus_int_unregister,
	.int_enable	= pcibus_int_enable,
	.int_disable	= pcibus_int_disable,
	.int_clear	= pcibus_int_clear,
	.int_mask	= NULL,
	.int_unmask	= NULL,
	.get_params	= pcibus_get_params,
	.freq_get	= pcibus_freq_get,
#ifdef PCIBUS_INFO
	.info_dev	= pcibus_dev_info,
#endif
};

/* Driver resources configuration for the PCI bus. It is declared weak so that
 * the user may override it from the project file, if the default settings are
 * not enough.
 */
struct rtems_drvmgr_bus_res pcibus_drv_resources __attribute__((weak)) =
{
	.next = NULL,
	.resource =
	{
		RES_EMPTY,
	},
};

struct pcibus_priv {
	struct rtems_drvmgr_dev_info	*dev;
};

static int compatible(struct pci_dev_id *id, struct pci_dev_id_match *drv)
{
	if (((drv->vendor==PCI_ID_ANY) || (id->vendor==drv->vendor)) &&
	    ((drv->device==PCI_ID_ANY) || (id->device==drv->device)) &&
	    ((drv->subvendor==PCI_ID_ANY) || (id->subvendor==drv->subvendor)) &&
	    ((drv->subdevice==PCI_ID_ANY) || (id->subdevice==drv->subdevice)) &&
	    ((id->class & drv->class_mask) == drv->class))
		return 1;
	else
		return 0;
}

int pcibus_unite(struct rtems_drvmgr_drv_info *drv,
			struct rtems_drvmgr_dev_info *dev)
{
	struct pci_drv_info *pdrv;
	struct pci_dev_id_match *drvid;
	struct pci_dev_info *pci;

	if (!drv || !dev || !dev->parent)
		return 0;

	if ((drv->bus_type != DRVMGR_BUS_TYPE_PCI) ||
	     (dev->parent->bus_type != DRVMGR_BUS_TYPE_PCI))
		return 0;

	pci = (struct pci_dev_info *)dev->businfo;
	if (!pci)
		return 0;

	pdrv = (struct pci_drv_info *)drv;
	drvid = pdrv->ids;
	if (!drvid)
		return 0;
	while (drvid->vendor != 0) {
		if (compatible(&pci->id, drvid)) {
			/* Unite device and driver */
			DBG("DRV %p and DEV %p united\n", drv, dev);
			return 1;
		}
		drvid++;
	}

	return 0;
}

static int pcibus_int_get(struct rtems_drvmgr_dev_info *dev, int index)
{
	int irq;

	/* Relative (positive) or absolute (negative) IRQ number */
	if (index > 0) {
		/* PCI devices only have one IRQ per function */
		return -1;
	} else if (index == 0) {
		/* IRQ Index relative to Cores base IRQ */

		/* Get Base IRQ */
		irq = ((struct pci_dev_info *)dev->businfo)->irq;
		if (irq <= 0)
			return -1;
	} else {
		/* Absolute IRQ number */
		irq = -index;
	}
	return irq;
}

/* Use standard PCI facility to register interrupt handler */
int pcibus_int_register(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct rtems_drvmgr_dev_info *busdev;
	struct ambapp_priv *priv;
	int irq;

	busdev = dev->parent->dev;
	priv = dev->parent->priv;

	/* Get IRQ number from index and device information */
	irq = pcibus_int_get(dev, index);
	if ( irq < 0 ) 
		return -1;

	DBG("Register PCI interrupt on 0x%x for dev 0x%x (IRQ: %d)\n",
		(unsigned int)busdev, (unsigned int)dev, irq);

	return pci_interrupt_register(irq, isr, arg);
}

/* Use standard PCI facility to unregister interrupt handler */
int pcibus_int_unregister(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct rtems_drvmgr_dev_info *busdev;
	struct ambapp_priv *priv;
	int irq;

	busdev = dev->parent->dev;
	priv = dev->parent->priv;

	/* Get IRQ number from index and device information */
	irq = pcibus_int_get(dev, index);
	if ( irq < 0 ) 
		return -1;

	DBG("Unregister PCI interrupt on 0x%x for dev 0x%x (IRQ: %d)\n",
		(unsigned int)busdev, (unsigned int)dev, irq);

	return pci_interrupt_unregister(irq, isr, arg);
}

/* Use standard PCI facility to enable interrupt */
int pcibus_int_enable(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg)
{
	int irq;

	/* Get IRQ number from index and device information */
	irq = pcibus_int_get(dev, index);
	if ( irq < 0 ) 
		return -1;

	return pci_interrupt_enable(irq, isr, arg);
}

/* Use standard PCI facility to disable interrupt */
int pcibus_int_disable(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg)
{
	int irq;

	/* Get IRQ number from index and device information */
	irq = pcibus_int_get(dev, index);
	if ( irq < 0 ) 
		return -1;

	return pci_interrupt_disable(irq, isr, arg);
}

/* Use standard PCI facility to clear interrupt */
int pcibus_int_clear(
	struct rtems_drvmgr_dev_info *dev,
	int index,
	rtems_drvmgr_isr isr,
	void *arg)
{
	int irq;

	/* Get IRQ number from index and device information */
	irq = pcibus_int_get(dev, index);
	if ( irq < 0 ) 
		return -1;

	return pci_interrupt_clear(irq, isr, arg);
}

int pcibus_freq_get(
	struct rtems_drvmgr_dev_info *dev,
	int options,
	unsigned int *freq_hz)
{
	/* Standard PCI Bus frequency */
	*freq_hz = 33000000;
	return 0;
}

int pcibus_get_params(struct rtems_drvmgr_dev_info *dev, struct rtems_drvmgr_bus_params *params)
{
	/* No device prefix */
	params->dev_prefix = NULL;

	return 0;
}

#ifdef PCIBUS_INFO
void pcibus_dev_info(
	struct rtems_drvmgr_dev_info *dev,
	void (*print_line)(void *p, char *str),
	void *p)
{
	struct pci_dev_info *devinfo;
	struct pcibus_res *pcibusres;
	struct pci_res *res;
	char buf[64];
	int i;
	char *str1, *res_types[3] = {" IO16", "  MEM", "MEMIO"};
	uint32_t pcistart;

	if (!dev)
		return;

	devinfo = (struct pci_dev_info *)dev->businfo;
	if (!devinfo)
		return;

	if ((devinfo->id.class >> 8) == PCI_CLASS_BRIDGE_PCI)
		print_line(p, "PCI BRIDGE DEVICE");
	else
		print_line(p, "PCI DEVICE");
	sprintf(buf, "LOCATION:    BUS:SLOT:FUNCTION [%x:%x:%x]",
			PCI_DEV_EXPAND(devinfo->pcidev));
	print_line(p, buf);
	sprintf(buf, "PCIID        0x%lx", (uint32_t)devinfo->pcidev);
	print_line(p, buf);
	sprintf(buf, "VENDOR ID:   %04x", devinfo->id.vendor);
	print_line(p, buf);
	sprintf(buf, "DEVICE ID:   %04x", devinfo->id.device);
	print_line(p, buf);
	sprintf(buf, "SUBVEN ID:   %04x", devinfo->id.subvendor);
	print_line(p, buf);
	sprintf(buf, "SUBDEV ID:   %04x", devinfo->id.subdevice);
	print_line(p, buf);
	sprintf(buf, "CLASS:       %lx", devinfo->id.class);
	print_line(p, buf);
	sprintf(buf, "REVISION:    %x", devinfo->rev);
	print_line(p, buf);
	sprintf(buf, "IRQ:         %d", devinfo->irq);
	print_line(p, buf);
	sprintf(buf, "PCIDEV ptr:  %p", devinfo->pci_device);
	print_line(p, buf);

	/* List Resources */
	print_line(p, "RESOURCES");
	for (i=0; i<PCIDEV_RES_CNT; i++) {
		pcibusres = &devinfo->resources[i];

		str1 = "  RES";
		pcistart = -1;
		res = pcibusres->res;
		if (res && (res->flags & PCI_RES_TYPE_MASK)) {
			str1 = res_types[(res->flags & PCI_RES_TYPE_MASK) - 1];
			if (res->flags & PCI_RES_IO32)
				str1 = " IO32";
			pcistart = res->start;
		}

		if (res && (res->flags & PCI_RES_FAIL)) {
			sprintf(buf, " %s[%d]:  NOT ASSIGNED", str1, i);
			print_line(p, buf);
			continue;
		}
		if (!pcibusres->size)
			continue;

		sprintf(buf, " %s[%d]:  %08lx-%08lx [PCIADR %lx]",
			str1, i, pcibusres->address,
			pcibusres->address + pcibusres->size - 1, pcistart);
		print_line(p, buf);
	}
}
#endif

#ifdef USE_PCI_CFG_LIB

int pcibus_dev_register(struct pci_dev *dev, void *arg)
{
	struct rtems_drvmgr_bus_info *pcibus = arg;
	struct rtems_drvmgr_dev_info *newdev;
	struct pci_dev_info *pciinfo;
	int i, type;
	struct pcibus_res *pcibusres;
	struct pci_res *pcires;

	pci_dev_t pcidev = dev->busdevfun;

	DBG("PCI DEV REGISTER: %x:%x:%x\n", PCI_DEV_EXPAND(pcidev));

	/* Allocate a device */
	rtems_drvmgr_alloc_dev(&newdev, 24 + sizeof(struct pci_dev_info));
	newdev->next = NULL;
	newdev->parent = pcibus; /* Ourselfs */
	newdev->minor_drv = 0;
	newdev->minor_bus = 0;
	newdev->priv = NULL;
	newdev->drv = NULL;
	newdev->name = (char *)(newdev + 1);
	newdev->next_in_drv = NULL;
	newdev->bus = NULL;

	/* Init PnP information, Assign Core interfaces with this device */
	pciinfo = (struct pci_dev_info *)((char *)(newdev + 1) + 24);

	/* Read Device and Vendor */
	pciinfo->id.vendor = dev->vendor;
	pciinfo->id.device = dev->device;
	pciinfo->id.subvendor = dev->subvendor;
	pciinfo->id.subdevice = dev->subdevice;
	pciinfo->rev = dev->classrev & 0xff;
	pciinfo->id.class = (dev->classrev >> 8) & 0xffffff;

	/* Read IRQ information set by PCI layer */
	pciinfo->irq = dev->sysirq;

	/* Save Location on PCI bus */
	pciinfo->pcidev = pcidev;

	/* Connect device with PCI data structure */
	pciinfo->pci_device = dev;

	/* Build resources so that PCI device drivers doesn't have to scan
	 * configuration space themselves, also the address is translated
	 * into CPU accessible addresses.
	 */
	for(i=0; i<PCIDEV_RES_CNT; i++) {
		pcibusres = &pciinfo->resources[i];
		pcires = &dev->resources[i];
		type = pcires->flags & PCI_RES_TYPE_MASK;
		if (type == 0 || (pcires->flags & PCI_RES_FAIL))
			continue; /* size=0 */

		pcibusres->address = pcires->start;
		if (pci_pci2cpu(&pcibusres->address, type))
			continue; /* size=0 */
		pcibusres->res = pcires;
		pcibusres->size = pcires->end - pcires->start;
	}

	/* Connect device with PCI information */
	newdev->businfo = (void *)pciinfo;

	/* Create Device Name */
	sprintf(newdev->name, "PCI_%x:%x:%x_%04x:%04x",
		PCI_DEV_BUS(pcidev), PCI_DEV_SLOT(pcidev), PCI_DEV_FUNC(pcidev),
		pciinfo->id.vendor, pciinfo->id.device);

	/* Register New Device */
	rtems_drvmgr_dev_register(newdev);

	/* If device is a bridge we scan the secondary (child) devices */
	if (dev->flags & PCI_DEV_BRIDGE) {
		pci_for_each_dev((struct pci_bus *)dev, pcibus_dev_register,
					pcibus);
	}

	return 0;
}

#else

int pcibus_dev_register(pci_dev_t pcidev, void *arg)
{
	struct rtems_drvmgr_bus_info *pcibus = arg;
	struct rtems_drvmgr_dev_info *newdev;
	struct pci_dev_info *pciinfo;

	DBG("PCI DEV REGISTER: %x:%x:%x\n", PCI_DEV_EXPAND(pcidev));

	/* Allocate a device */
	rtems_drvmgr_alloc_dev(&newdev, 24 + sizeof(struct pci_dev_info));
	newdev->next = NULL;
	newdev->parent = pcibus; /* Ourselfs */
	newdev->minor_drv = 0;
	newdev->minor_bus = 0;
	newdev->priv = NULL;
	newdev->drv = NULL;
	newdev->name = (char *)(newdev + 1);
	newdev->next_in_drv = NULL;
	newdev->bus = NULL;

	/* Init PnP information, Assign Core interfaces with this device */
	pciinfo = (struct pci_dev_info *)((char *)(newdev + 1) + 24);

	/* Read Device and Vendor */
	pci_cfg_r16(pcidev, PCI_VENDOR_ID, &pciinfo->id.vendor);
	pci_cfg_r16(pcidev, PCI_DEVICE_ID, &pciinfo->id.device);
	pci_cfg_r32(pcidev, PCI_CLASS_REVISION, &pciinfo->id.class);
	pciinfo->rev = pciinfo->id.class & 0xff;
	pciinfo->id.class = pciinfo->id.class >> 8;

	/* Devices have subsytem device and vendor ID */
	if ((pciinfo->id.class >> 8) != PCI_CLASS_BRIDGE_PCI) {
		pci_cfg_r16(pcidev, PCI_SUBSYSTEM_VENDOR_ID,
							&pciinfo->id.subvendor);
		pci_cfg_r16(pcidev, PCI_SUBSYSTEM_ID, &pciinfo->id.subdevice);
	} else {
		pciinfo->id.subvendor = 0;
		pciinfo->id.subdevice = 0;
	}

	/* Read IRQ information set by PCI layer */
	pci_cfg_r8(pcidev, PCI_INTERRUPT_LINE, &pciinfo->irq);

	/* Save Location */
	pciinfo->pcidev = pcidev;

	/* There is no way we can know this information this way */
	pciinfo->pci_device = NULL;

	/* Connect device with PCI information */
	newdev->businfo = (void *)pciinfo;

	/* Create Device Name */
	sprintf(newdev->name, "PCI_%d:%d:%d_%04x:%04x",
		PCI_DEV_BUS(pcidev), PCI_DEV_SLOT(pcidev), PCI_DEV_FUNC(pcidev),
		pciinfo->id.vendor, pciinfo->id.device);

	/* Register New Device */
	rtems_drvmgr_dev_register(newdev);

	return 0;
}

#endif

/* Register all AMBA devices available on the AMBAPP bus */
static int pcibus_devs_register(struct rtems_drvmgr_bus_info *bus)
{
#ifdef USE_PCI_CFG_LIB
	pci_for_each_dev(&pci_hb, pcibus_dev_register, bus);
#else
	pci_for_each(pcibus_dev_register, bus);
#endif
	return DRVMGR_OK;
}

/*** DEVICE FUNCTIONS ***/

int pcibus_register(struct rtems_drvmgr_dev_info *dev)
{
	struct pcibus_priv *priv;

	DBG("PCI BUS: initializing\n");

	/* Create BUS */
	rtems_drvmgr_alloc_bus(&dev->bus, sizeof(struct pcibus_priv));
	dev->bus->bus_type = DRVMGR_BUS_TYPE_PCI;
	dev->bus->next = NULL;
	dev->bus->dev = dev;
	dev->bus->children = NULL;
	dev->bus->ops = &pcibus_ops;
	dev->bus->dev_cnt = 0;
	dev->bus->reslist = NULL;
	dev->bus->mmaps = NULL;

	/* Add resource configuration if user overrided the default empty cfg */
	if (pcibus_drv_resources.resource[0].drv_id != 0)
		rtems_drvmgr_bus_res_add(dev->bus, &pcibus_drv_resources);

	/* Init BUS private structures */
	priv = (struct pcibus_priv *)(dev->bus + 1);
	dev->bus->priv = priv;	

	/* Register BUS */
	rtems_drvmgr_bus_register(dev->bus);

	return DRVMGR_OK;
}

/*** BUS INITIALIZE FUNCTIONS ***/

int pcibus_bus_init1(struct rtems_drvmgr_bus_info *bus)
{
	return pcibus_devs_register(bus);
}
