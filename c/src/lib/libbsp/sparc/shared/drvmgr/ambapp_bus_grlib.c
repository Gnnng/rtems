/*  LEON3 GRLIB AMBA Plug & Play bus driver.
 *
 *  COPYRIGHT (c) 2008.
 *  Aeroflex Gaisler.
 *
 *  This is driver is a wrapper for the general AMBA Plug & Play bus
 *  driver. This is the root bus driver for GRLIB systems.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  2008-12-03, Daniel Hellstrom <daniel@gaisler.com>
 *    Created
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drvmgr/ambapp_bus.h>
#include <drvmgr/ambapp_bus_grlib.h>
#include <genirq.h>

#include <bsp.h>

#define DBG(args...)
/*#define DBG(args...) printk(args)*/

int ambapp_grlib_int_register(
	struct drvmgr_dev *dev,
	int irq,
	const char *info,
	drvmgr_isr isr,
	void *arg);
int ambapp_grlib_int_unregister(
	struct drvmgr_dev *dev,
	int irq,
	drvmgr_isr isr,
	void *arg);
int ambapp_grlib_int_clear(
	struct drvmgr_dev *dev,
	int irq);
int ambapp_grlib_int_mask(
	struct drvmgr_dev *dev,
	int irq);
int ambapp_grlib_int_unmask(
	struct drvmgr_dev *dev,
	int irq);
int ambapp_grlib_get_params(
	struct drvmgr_dev *dev,
	struct drvmgr_bus_params *params);

int ambapp_grlib_init1(struct drvmgr_dev *dev);
int ambapp_grlib_init2(struct drvmgr_dev *dev);
int ambapp_grlib_remove(struct drvmgr_dev *dev);

struct ambapp_ops ambapp_grlib_ops = {
	.int_register = ambapp_grlib_int_register,
	.int_unregister = ambapp_grlib_int_unregister,
	.int_clear = ambapp_grlib_int_clear,
	.int_mask = ambapp_grlib_int_mask,
	.int_unmask = ambapp_grlib_int_unmask,
	.get_params = ambapp_grlib_get_params
};

struct drvmgr_drv_ops ambapp_grlib_drv_ops = 
{
	.init = {ambapp_grlib_init1, ambapp_grlib_init2, NULL, NULL},
	.remove = ambapp_grlib_remove,
	.info = NULL,
};

struct drvmgr_drv ambapp_bus_drv_grlib = 
{
	DRVMGR_OBJ_DRV,			/* Driver */
	NULL,				/* Next driver */
	NULL,				/* Device list */
	DRIVER_GRLIB_AMBAPP_ID,		/* Driver ID */
	"AMBAPP_GRLIB_DRV",		/* Driver Name */
	DRVMGR_BUS_TYPE_ROOT,		/* Bus Type */
	&ambapp_grlib_drv_ops,
	0,
	0,
};

static struct grlib_config *drv_mgr_grlib_config = NULL;

void ambapp_grlib_register(void)
{
	drvmgr_drv_register(&ambapp_bus_drv_grlib);
}

int drv_mgr_grlib_init(struct grlib_config *config)
{

	/* Save the configuration for later */
	drv_mgr_grlib_config = config;

	/* Register root device driver */
	drvmgr_root_drv_register(&ambapp_bus_drv_grlib);

	return 0;
}

/* Function called from Driver Manager Initialization Stage 1 */
int ambapp_grlib_init1(struct drvmgr_dev *dev)
{
	struct ambapp_config *config;

	dev->priv = NULL;
	dev->name = "GRLIB AMBA PnP";

	DBG("AMBAPP GRLIB: intializing\n");

	config = malloc(sizeof(struct ambapp_config));
	if ( !config )
		return RTEMS_NO_MEMORY;

	config->ops = &ambapp_grlib_ops;
	config->mmaps = NULL;
	config->abus = drv_mgr_grlib_config->abus;
	config->resources = drv_mgr_grlib_config->resources;

	/* Initialize the generic part of the AMBA Bus */
	return ambapp_bus_register(dev, config);
}

int ambapp_grlib_init2(struct drvmgr_dev *dev)
{
	return 0;
}

int ambapp_grlib_remove(struct drvmgr_dev *dev)
{
	return 0;
}

int ambapp_grlib_int_register
	(
	struct drvmgr_dev *dev,
	int irq,
	const char *info,
	drvmgr_isr isr,
	void *arg
	)
{
	return BSP_shared_interrupt_register(irq, info, isr, arg);
}

int ambapp_grlib_int_unregister
	(
	struct drvmgr_dev *dev,
	int irq,
	drvmgr_isr isr,
	void *arg
	)
{
	return BSP_shared_interrupt_unregister(irq, isr, arg);
}

int ambapp_grlib_int_clear
	(
	struct drvmgr_dev *dev,
	int irq)
{
	BSP_shared_interrupt_clear(irq);
	return DRVMGR_OK;
}

int ambapp_grlib_int_mask
	(
	struct drvmgr_dev *dev,
	int irq
	)
{
	BSP_shared_interrupt_mask(irq);
	return DRVMGR_OK;
}

int ambapp_grlib_int_unmask
	(
	struct drvmgr_dev *dev,
	int irq
	)
{
	BSP_shared_interrupt_unmask(irq);
	return DRVMGR_OK;
}

int ambapp_grlib_get_params(struct drvmgr_dev *dev, struct drvmgr_bus_params *params)
{
	/* Leave params->freq_hz untouched for default */
	params->dev_prefix = "";
	return 0;
}
