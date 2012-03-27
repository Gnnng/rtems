/*  PCI Help function, Find a PCI device by VENDOR/DEVICE ID
 *
 *  COPYRIGHT (c) 2010-2011.
 *  Aeroflex Gaisler Research
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  2011-02-12, Daniel Hellstrom <daniel@gaisler.com>
 *    new implementation based on pci_for_each() to reduce footprint
 */

#include <pci.h>
#include <pci/access.h>

struct compare_info {
	int index;
	uint16_t vendor;
	uint16_t device;
};

static int compare_dev_id(pci_dev_t pcidev, void *arg)
{
	struct compare_info *info = arg;
	uint16_t vid, did;

	pci_cfg_r16(pcidev, PCI_VENDOR_ID, &vid);
	pci_cfg_r16(pcidev, PCI_DEVICE_ID, &did);
	if ((vid != info->vendor) || (did != info->device))
		return 0;
	if (info->index-- == 0)
		return pcidev;
	else
		return 0;
}

/* Find a Device in PCI configuration space */
int pci_find(uint16_t ven, uint16_t dev, int index, pci_dev_t *pdev)
{
	struct compare_info info;
	int result;

	info.index = index;
	info.vendor = ven;
	info.device = dev;

	result = pci_for_each(compare_dev_id, &info);
	if (pdev)
		*pdev = (pci_dev_t)result;
	if (result == 0)
		return -1;
	else
		return 0;
}
