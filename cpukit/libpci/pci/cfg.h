/* PCI Configuration Library, two versions of the library exists:
 *  - auto configuration (default)
 *  - static configuration (user defined config)
 * both versions are defined here.
 *
 */

#ifndef __PCI_CFG_H__
#define __PCI_CFG_H__

#include <pci.h>

/* PCI Configuration library */

/* Return the number of PCI buses in system */
extern int pci_bus_count(void);

/* PCI Address assigned to BARs which failed to fit into the PCI Window or
 * is disabled by any other cause.
 */
extern uint32_t pci_invalid_address;

/* PCI System type can be used to determine system for drivers. Normally
 * the system is Host, but the peripheral configuration library also supports
 * being PCI peripheral not allowed to access configuration space.
 *
 * The active configuration Library set this variable.
 */
enum {
	PCI_SYSTEM_NONE = 0,
	PCI_SYSTEM_HOST = 1,
	PCI_SYSTEM_PERIPHERAL = 2,
};
extern int pci_system_type;

/* Configuration library function pointers, these are set in <rtems/confdefs.h>
 * by project configuration or by the BSP. The configuration will pull in the
 * PCI Library needed and the PCI initialization functions will call these
 * functions on initialization from the host driver.
 */
extern int (*pci_config_lib_init)(void);
extern void (*pci_config_lib_register)(void *config);

/* Configure PCI devices and bridges, and setup the RAM data structures
 * describing the PCI devices currently present in the system.
 *
 * Returns 0 on success, -1 on failure.
 */
extern int pci_config_init(void);

/* Register a config-library specific configuration used by the libarary in
 * pci_config_init().
 */
extern void pci_config_register(void *config);

/* Print current PCI configuration (C-code) to terminal, can be used in
 * static and peripheral PCI configuration library. The configuration is
 * taken from the current configuration library setup.
 */
extern void pci_cfg_print(void);

struct pci_bus; /* Bridge Device and secondary bus information */
struct pci_dev; /* Device/function */
struct pci_res; /* Resource: BAR, ROM or Bridge Window */

/* The Host Bridge and all subdevices (the PCI RAM data structure) */
extern struct pci_bus pci_hb;

/* Iterate over all PCI devices on a bus (not child buses) and call func(),
 * iteration is stopped if a non-zero value is returned by func().
 *
 * The function iterates over the PCI RAM data structure, it is not
 * available until after all devices have been found and pci_hb is populated,
 * typcially after pci_config_init() is called.
 */
extern int pci_for_each_dev(
	struct pci_bus *bus,
	int (*func)(struct pci_dev *, void *arg),
	void *arg);

/* Resource flags */
#define PCI_RES_IO 1
#define PCI_RES_MEMIO 2
#define PCI_RES_MEM_PREFETCH 1
#define PCI_RES_MEM (PCI_RES_MEMIO | PCI_RES_MEM_PREFETCH)
#define PCI_RES_TYPE_MASK 0x3
#define PCI_RES_IO32 0x08
#define PCI_RES_FAIL 0x10 /* Alloc Failed */

/* BAR Resouces entry */
struct pci_res {
	struct pci_res	*next;
	uint32_t	size;
	uint32_t	boundary;
	unsigned char	flags; /* I/O, MEM or MEMIO */
	unsigned char	bar;

	/* Assigned Resource (PCI address), zero if not assigned */
	uint32_t	start;
	uint32_t	end;
};

/* Get Device from resource pointer */
#define RES2DEV(res) ((struct pci_dev *) \
			((void *)res - (res->bar * (sizeof(struct pci_res)))))

/* Device flags */
#define PCI_DEV_BRIDGE    0x01  /* Device is a Bridge (struct pci_bus) */
#define PCI_DEV_RES_FAIL  0x02  /* Resource alloction for device BARs failed */

/* Bus Flags */
#define PCI_BUS_IO        0x01	/* 16-bit I/O address decoding */
#define PCI_BUS_MEMIO     0x02  /* Bus support non-prefetchable mem (always) */
#define PCI_BUS_MEM       0x04  /* Bus support prefetchable memory space */
#define PCI_BUS_IO32      0x08	/* 32-bit I/O address decoding */

#define BRIDGE_RES_COUNT 2 /* Number of BAR resources a bridge can have */
#define BUS_RES_START BRIDGE_RES_COUNT

/* Bus Resources Array */
enum {
	BUS_RES_IO = 0,
	BUS_RES_MEMIO = 1,
	BUS_RES_MEM = 2,
};

/* Device Resource array index meaning */
enum {
	/* A Device has up to 6 BARs and an optional ROM BAR */
	DEV_RES_BAR1 = 0,
	DEV_RES_BAR2 = 1,
	DEV_RES_BAR3 = 2,
	DEV_RES_BAR4 = 3,
	DEV_RES_BAR5 = 4,
	DEV_RES_BAR6 = 5,
	DEV_RES_ROM  = 6,

	/* Bridges have 2 BARs (BAR1 and BAR2) and 3 Windows to secondary bus
	 * and an optional ROM BAR
	 */
	BRIDGE_RES_BAR1 = 0,
	BRIDGE_RES_BAR2 = 1,
	BRIDGE_RES_IO = 2,
	BRIDGE_RES_MEMIO = 3,
	BRIDGE_RES_MEM = 4,
	BRIDGE_RES_UNUSED1 = 5,
	BRIDGE_RES_ROM = 6,
};

/* Maximum Number of Resources of a device */
#define DEV_RES_CNT (DEV_RES_ROM + 1)

/* PCI Device (Bus|Slot|Function) description */
struct pci_dev {
	struct pci_res	resources[DEV_RES_CNT]; /* must be topmost field */
	struct pci_dev	*next;
	struct pci_bus	*bus;
	pci_dev_t	busdevfun;
	uint8_t		flags;
	uint8_t		sysirq;
	uint16_t	vendor;
	uint16_t	device;
	uint16_t	subvendor;
	uint16_t	subdevice;
	uint32_t	classrev;

	/* static configuration settings */
	uint16_t	command;
};

/* PCI Bus description */
struct pci_bus {
	struct pci_dev	dev; /* PCI Bridge */
	struct pci_dev	*devs; /* Devices on child (secondary) Bus */
	unsigned int	flags;

	/* Bridge Information */
	int num;	/* Bus number (0=Root-PCI-bus) */
	int pri;	/* Primary Bus Number */
	int sord;	/* Subordinate Buses (Child bus count) */

#if defined(PCI_CFG_AUTO_LIB)
	/* Resources of devices on bus. USED INTERNALLY IN AUTO-CFG LIBRARY.
	 *
	 * BUS_RES_IO    = 0:  I/O resources
	 * BUS_RES_MEMIO = 1:  Prefetchable memory resources
	 * BUS_RES_MEM   = 2:  Non-Prefetchable memory resources
	 */
	struct pci_res	*busres[3];
#endif
};

#include <pci/cfg_auto.h>
#include <pci/cfg_static.h>
#include <pci/cfg_read.h>
#include <pci/cfg_peripheral.h>

#endif
