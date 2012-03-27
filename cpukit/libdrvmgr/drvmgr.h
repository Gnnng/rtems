/*  Driver Manager Interface.
 *
 *  COPYRIGHT (c) 2009.
 *  Aeroflex Gaisler AB
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *
 *
 * Define this to make the driver manager do Breath First "searching" when 
 * initializing devices on buses. If not defined depth first will be used.
 */

#ifndef _DRIVER_MANAGER_H_
#define _DRIVER_MANAGER_H_

#include <rtems.h>
#include <drvmgr/drvmgr_list.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*** Configure Driver manager ***/

/* Define the number of initialization levels of device drivers */
#define DRVMGR_LEVEL_MAX 4

struct drvmgr_dev;	/* Device */
struct drvmgr_bus;	/* Bus */
struct drvmgr_drv;	/* Driver */

/*** List Interface shortcuts ***/
#define BUS_LIST_HEAD(list) LIST_HEAD(list, struct drvmgr_bus)
#define BUS_LIST_TAIL(list) LIST_TAIL(list, struct drvmgr_bus)
#define DEV_LIST_HEAD(list) LIST_HEAD(list, struct drvmgr_dev)
#define DEV_LIST_TAIL(list) LIST_TAIL(list, struct drvmgr_dev)
#define DRV_LIST_HEAD(list) LIST_HEAD(list, struct drvmgr_drv)
#define DRV_LIST_TAIL(list) LIST_TAIL(list, struct drvmgr_drv)

/*** Bus indentification ***/
#define DRVMGR_BUS_TYPE_NONE 0		/* Not a valid bus */
#define DRVMGR_BUS_TYPE_ROOT 1		/* Hard coded bus */
#define DRVMGR_BUS_TYPE_PCI 2		/* PCI bus */
#define DRVMGR_BUS_TYPE_AMBAPP 3	/* AMBA Plug & Play bus */
#define DRVMGR_BUS_TYPE_LEON2_AMBA 4	/* LEON2 hardcoded bus */
#define DRVMGR_BUS_TYPE_AMBAPP_DIST 5	/* Distibuted AMBA Plug & Play bus accessed using a communication interface */
#define DRVMGR_BUS_TYPE_SPW_RMAP 6	/* SpaceWire Network bus */
#define DRVMGR_BUS_TYPE_AMBAPP_RMAP 7	/* SpaceWire RMAP accessed AMBA Plug & Play bus */

enum {
 DRVMGR_OBJ_NONE = 0,
 DRVMGR_OBJ_DRV = 1,
 DRVMGR_OBJ_BUS = 2,
 DRVMGR_OBJ_DEV = 3,
};

/*** Driver indentification ***/ 

/* 64-bit identification integer definition
 *  � Bus ID 8-bit [7..0]
 *  � Reserved 8-bit field [63..56]
 *  � Device ID specific for bus type 48-bit [55..8]  (Different buses have different unique identifications for hardware/driver.)
 *
 * ID Rules
 *  � A root bus driver must always have device ID set to 0. There can only by one root bus driver
 *    for a certain bus type.
 *  � A Driver ID must identify a unique hardware core
 *
 */

/* Bus ID Mask */
#define DRIVER_ID_BUS_MASK 0x00000000000000FFULL

/* Reserved Mask for future use */
#define DRIVER_ID_RSV_MASK 0xFF00000000000000ULL

/* Reserved Mask for future use */
#define DRIVER_ID_DEV_MASK 0x00FFFFFFFFFFFF00ULL

/* Set Bus ID Mask. */
#define DRIVER_ID(busid, devid) \
	(unsigned long long)( (((unsigned long long)(devid) << 8) & DRIVER_ID_DEV_MASK) | ((unsigned long long)(busid) & DRIVER_ID_BUS_MASK) )

/* Get IDs */
#define DRIVER_BUSID_GET(id)		((unsigned long long)(id) & DRIVER_ID_BUS_MASK)
#define DRIVER_DEVID_GET(id)		(((unsigned long long)(id) & DRIVER_ID_DEV_MASK) >> 8)

#define DRIVER_ROOTBUS_ID(bus_type)	DRIVER_ID(bus_type, 0)

/*** Root Bus drivers ***/

/* Generic Hard coded Root bus: Driver ID */
#define DRIVER_ROOT_ID			DRIVER_ROOTBUS_ID(DRVMGR_BUS_TYPE_ROOT)

/* PCI Plug & Play bus: Driver ID */
#define DRIVER_PCIBUS_ID		DRIVER_ROOTBUS_ID(DRVMGR_BUS_TYPE_PCI)

/* AMBA Plug & Play bus: Driver ID */
#define DRIVER_GRLIB_AMBAPP_ID		DRIVER_ROOTBUS_ID(DRVMGR_BUS_TYPE_AMBAPP)

/* AMBA Hard coded bus: Driver ID */
#define DRIVER_LEON2_AMBA_ID		DRIVER_ROOTBUS_ID(DRVMGR_BUS_TYPE_LEON2_AMBA)

/* Distributed AMBA Plug & Play bus: Driver ID */
#define DRIVER_AMBAPP_DIST_ID		DRIVER_ROOTBUS_ID(DRVMGR_BUS_TYPE_AMBAPP_DIST)

/*! Bus parameters used by driver interface functions to aquire information about 
 * bus. All Bus drivers should implement the operation 'get_params' so that the
 * driver interface routines can access bus dependent information in an non-dependent 
 * way.
 */
struct drvmgr_bus_params {
#if 0
	unsigned int	freq_hz;		/*!< Frequency of bus in Hz */
#endif
	char		*dev_prefix;		/*!<  */
};

/* Interrupt Service Routine (ISR) */
typedef void (*drvmgr_isr)(void *arg);

/*! Bus operations */
struct drvmgr_bus_ops {
	/* Functions used internally within driver manager */
	int	(*init[DRVMGR_LEVEL_MAX])(struct drvmgr_bus *);
	int	(*remove)(struct drvmgr_bus *);
	int	(*unite)(struct drvmgr_drv *, struct drvmgr_dev *);	/*!< Unite Hardware Device with Driver */

	/* Functions called indirectly from drivers */
	int	(*int_register)(struct drvmgr_dev *, int index, const char *info, drvmgr_isr isr, void *arg);
	int	(*int_unregister)(struct drvmgr_dev *, int index, drvmgr_isr isr, void *arg);
	int	(*int_clear)(struct drvmgr_dev *, int index);
	int	(*int_mask)(struct drvmgr_dev *, int index);
	int	(*int_unmask)(struct drvmgr_dev *, int index);

	/* Get Paramters */
	int	(*get_params)(struct drvmgr_dev *, struct drvmgr_bus_params *);
	/* Get Frequency of Bus */
	int	(*freq_get)(struct drvmgr_dev*, int, unsigned int*);
	/*! Function called to request information about a device. The bus
	 *  driver interpret the bus-specific information about the device.
	 */
	void	(*info_dev)(struct drvmgr_dev *, void (*print)(void *p, char *str), void *p);
};
#define BUS_OPS_NUM (sizeof(struct drvmgr_bus_ops)/sizeof(void (*)(void)))

struct drvmgr_func {
	int funcid;
	void *func;
};
#define DRVMGR_FUNC(_ID_, _FUNC_) {(int)(_ID_), (void *)(_FUNC_)}
#define DRVMGR_FUNC_END {0, NULL}

/*** Resource definitions ***
 * 
 * Overview of structures:
 *  All bus resources entries (_bus_res) are linked together per bus (bus_info->reslist).
 *  One bus resource entry has a pointer to an array of driver resources (_drv_res). One driver 
 *  resouces is made out of an array of keys (drvmgr_key). All keys belongs to the 
 *  same driver and harwdare device. Each key has a Name, Type ID and Data interpreted 
 *  differently depending on the Type ID (union drvmgr_key_value).
 *
 */

/* Key Data Types */
#define KEY_TYPE_NONE		0
#define KEY_TYPE_INT 		1
#define KEY_TYPE_STRING		2
#define KEY_TYPE_POINTER	3

#define KEY_EMPTY	{NULL, KEY_TYPE_NONE, {0}}
#define RES_EMPTY	{0, 0, NULL}
#define MMAP_EMPTY	{0, 0, 0}

/*! Union of different values */
union drvmgr_key_value {
	unsigned int		i;		/*!< Key data type UNSIGNED INTEGER */
	char			*str;		/*!< Key data type STRING */
	void			*ptr;		/*!< Key data type ADDRESS/POINTER */
};

/* One key. One Value. Holding information relevant to the driver. */
struct drvmgr_key {
	char			*key_name;	/* Name of key */
	int			key_type;	/* How to interpret key_value */
	union drvmgr_key_value	key_value;	/* The value or pointer to the value */
};

/*! Driver resource entry, Driver resources for a certain device instance, containing a number of keys 
 * Where each key hold the data of interest.
 */
struct drvmgr_drv_res {
	uint64_t		drv_id;		/*!< Identifies the driver this resource is aiming */
	int			minor_bus;	/*!< Indentifies a specfic device */
	struct drvmgr_key	*keys;		/*!< First key in key array, ended with KEY_EMPTY */
};

/*! Bus resource list node */
struct drvmgr_bus_res {
	struct drvmgr_bus_res	*next;		/*!< Next resource node in list */
	struct drvmgr_drv_res	resource[];	/*!< Array of resources, one per device instance */
};

/*! MAP entry. Describes an linear address space translation. Untranslated
 *  Start, Translated Start and length.
 *
 * Used by bus drivers to describe the address translation needed for 
 * the translation driver interface.
 */
struct drvmgr_mmap_entry {
	unsigned int	map_size;	/*!< Size of memory Window */
	char		*local_adr;	/*!< Start address of Window from CPU's 
					 * point of view */
	char		*remote_adr;	/*!< Start address of Remote system
					 * point of view */
};

/*! Bus information. Describes a bus. */
struct drvmgr_bus {
	int			obj_type;	/*!< DRVMGR_OBJ_BUS */
	int			bus_type;	/*!< Type of bus */
	struct drvmgr_bus	*next;		/*!< Next Bus */
	struct drvmgr_dev	*dev;		/*!< Bus device, the hardware... */
	void			*priv;		/*!< Private data structure used by BUS driver */
	struct drvmgr_dev	*children;	/*!< Hardware devices on this bus */
	struct drvmgr_bus_ops	*ops;		/*!< Bus operations supported by this bus driver */
	struct drvmgr_func	*funcs;		/*!< Extra operations */
	int			dev_cnt;	/*!< Number of devices this bus has */
	struct drvmgr_bus_res	*reslist;	/*!< Bus resources, head of a linked list of resources. */
	struct drvmgr_mmap_entry	*mmaps;		/*!< Memory Map Translation, array of address spaces */

	/* Bus status */
	int			level;
	int			state;
	int			error;
};

/* States of a bus */
#define BUS_STATE_INIT_FAILED	0x00000001	/* Initialization Failed */
#define BUS_STATE_LIST_INACTIVE	0x00001000	/* In inactive bus list */
#define BUS_STATE_DEPEND_FAILED	0x00000004	/* Device init failed */

/* States of a device */
#define DEV_STATE_INIT_FAILED	0x00000001	/* Initialization Failed */
#define DEV_STATE_INIT_DONE	0x00000002	/* All init levels completed */
#define DEV_STATE_DEPEND_FAILED	0x00000004	/* Parent Bus init failed */
#define DEV_STATE_UNITED	0x00000100	/* Device United with Device Driver */
#define DEV_STATE_REMOVED	0x00000200	/* Device has been removed (unregistered) */
#define DEV_STATE_IGNORED	0x00000400	/* Device was ignored according to user's request, the device
						 * was never reported to it's driver (as expected).
						 */
#define DEV_STATE_LIST_INACTIVE	0x00001000	/* In inactive device list */

/*! Device information */
struct drvmgr_dev {
	int			obj_type;	/*!< DRVMGR_OBJ_DEV */
	struct drvmgr_dev	*next;		/*!< Next device */
	struct drvmgr_dev	*next_in_bus;	/*!< Next device on the same bus */
	struct drvmgr_dev	*next_in_drv;	/*!< Next device using the same driver */

	struct drvmgr_drv	*drv;		/*!< The driver owning this device */
	struct drvmgr_bus	*parent;	/*!< Bus that this device resides on */
	short			minor_drv;	/*!< Device number on driver (often minor in filesystem) */
	short			minor_bus;	/*!< Device number on bus (for device separation) */
	char			*name;		/*!< Name of Device Hardware */
	void			*priv;		/*!< Pointer to driver private device structure */
	void			*businfo;	/*!< Host bus specific information */
	struct drvmgr_bus	*bus;		/*!< Pointer to bus, set only if this is a bus */

	/* Device Status */
	unsigned int		state;		/*!< State of this device, see DEV_STATE_* */
	int			level;		/*!< Init Level */
	int			error;		/*!< Error state returned by driver */
};

/*! Driver operations, function pointers. */
struct drvmgr_drv_ops {
	int	(*init[DRVMGR_LEVEL_MAX])(struct drvmgr_dev *);	/*! Function doing Init Stage 1 of a hardware device */
	int	(*remove)(struct drvmgr_dev *);	/*! Function called when device instance is to be removed */
	int	(*info)(struct drvmgr_dev *, void (*print)(void *p, char *str), void *p, int, char *argv[]);/*! Function called to request information about a device or driver */
};
#define DRV_OPS_NUM (sizeof(struct drvmgr_drv_ops)/sizeof(void (*)(void)))

/*! Information about a driver used during registration */
struct drvmgr_drv {
	int			obj_type;	/*!< DRVMGR_OBJ_DRV */
	struct drvmgr_drv	*next;		/*!< Next Driver */
	struct drvmgr_dev	*dev;		/*!< Devices using this driver */

	uint64_t		drv_id;		/*!< Unique Driver ID */
	char			*name;		/*!< Name of Driver */
	int			bus_type;	/*!< Type of Bus this driver supports */
	struct drvmgr_drv_ops	*ops;		/*!< Driver operations */
	struct drvmgr_func	*funcs;		/*!< Extra Operations */
	unsigned int		dev_cnt;	/*!< Number of devices in dev */
	unsigned int		dev_priv_size;	/*!< If non-zero DRVMGR will allocate memory for dev->priv */
};

/*! Structure defines a function pointer called when driver manager is ready for drivers to register
 *  themselfs. Used to select drivers available to the driver manager.
 */
struct drvmgr_drv_reg_func {
	void (*drv_reg)(void);
};

/*** DRIVER | DEVICE | BUS FUNCTIONS ***/

/* Return Codes */
enum {
	DRVMGR_OK = 0,
	DRVMGR_NOMEM = 1,
	DRVMGR_EIO = 2,
	DRVMGR_EINVAL = 3,
	DRVMGR_ENOSYS = 4,
	DRVMGR_TIMEDOUT = 5,
	DRVMGR_EBUSY = 6,
	DRVMGR_ENORES = 7,	/* Not enough resources */
	DRVMGR_FAIL = -1
};

/*! Initialize data structures of the driver management system. 
 *  Calls predefined register driver functions so that drivers can
 *  register themselves.
 */
extern void _DRV_Manager_initialization(void);

/*! Take all devices into init level 'level', all devices registered later
 *  will directly be taken into this level as well, ensuring that all
 *  registerd devices has been taken into the level.
 *
 */
extern void _DRV_Manager_init_level(int level);

/*! Init driver manager all in one go, will call _DRV_Manager_initialization(),
 *  then _DRV_Manager_init_level([1..DRVMGR_LEVEL_MAX]).
 *  Typically called from Init task when user wants to initilize driver
 *  manager after startup, otherwise not used.
 */
extern int drvmgr_init(void);

/* Take registered buses and devices into the correct init level,
 * this function is called from _init_level() so normally
 * we don't need to call it directly.
 */
extern void drvmgr_init_update(void);

/*! Register Root Bus device driver */
extern int drvmgr_root_drv_register(struct drvmgr_drv *drv);

/*! Register a driver */
extern int drvmgr_drv_register(struct drvmgr_drv *drv);

/*! Register a device */
extern int drvmgr_dev_register(struct drvmgr_dev *dev);

/*! Remove a device, and all its children devices if device is a bus device. The device 
 *  driver will be requested to remove the device and once gone from bus, device and 
 *  driver list the device is put into a inactive list for debugging (this is optional
 *  by using remove argument).
 *
 * Removing the Root Bus Device is not supported.
 *
 * \param remove If non-zero the device will be deallocated, and not put into the 
 *               inacitve list.
 */
extern int drvmgr_dev_unregister(struct drvmgr_dev *dev);

/*! Register a bus */
extern int drvmgr_bus_register(struct drvmgr_bus *bus);

/*! Unregister a bus */
extern int drvmgr_bus_unregister(struct drvmgr_bus *bus);

/*! Allocate a device structure, if no memory available rtems_error_fatal_occurred 
 * is called.
 * The 'extra' argment tells how many bytes extra space is to be allocated after
 * the device structure, this is typically used for "businfo" structures. The extra
 * space is always aligned to a 4-byte boundary.
 */
extern int drvmgr_alloc_dev(struct drvmgr_dev **pdev, int extra);

/*! Allocate a bus structure, if no memory available rtems_error_fatal_occurred 
 * is called.
 * The 'extra' argment tells how many bytes extra space is to be allocated after
 * the device structure, this is typically used for "businfo" structures. The extra
 * space is always aligned to a 4-byte boundary.
 */
extern int drvmgr_alloc_bus(struct drvmgr_bus **pbus, int extra);

/*** DRIVER RESOURCE FUNCTIONS ***/

/*! Add resources to a bus, typically used by a bus driver.
 *
 * \param bus   The Bus to add the resources to.
 * \param res   An array with Driver resources, all together are called bus resources.
 */
extern void drvmgr_bus_res_add(struct drvmgr_bus *bus,
					struct drvmgr_bus_res *bres);

/*! Find all the resource keys for a device among all driver resources on a bus. Typically
 *  used by a device driver to get configuration options.
 *
 * \param dev   Device to find resources for
 * \param key   Location where the pointer to the driver resource array (drvmgr_drv_res->keys) is stored.
 */
extern int drvmgr_keys_get(struct drvmgr_dev *dev, struct drvmgr_key **keys);

/*! Return the one key that matches key name from a driver keys array. The keys can be obtained
 *  using drvmgr_keys_get.
 *  
 * \param keys       An array of keys ended with KEY_EMPTY to search among.
 * \param key_name   Name of key to search for among the keys.
 */
extern struct drvmgr_key *drvmgr_key_get(struct drvmgr_key *keys, char *key_name);

/*! Extract key value from the key in the keys array matching name and type.
 *
 *  This function calls drvmgr_keys_get to get the key requested (from key name), then determines
 *  if the type is correct. A pointer to the key value is returned.
 *  
 *  \param keys       An array of keys ended with KEY_EMPTY to search among.
 *  \param key_name   Name of key to search for among the keys.
 *  \param key_type   Data Type of value. INTEGER, ADDRESS, STRING.
 *  \return           Returns NULL if no value found matching Key Name and Key Type.
 */
extern union drvmgr_key_value *drvmgr_key_val_get(
	struct drvmgr_key *keys,
	char *key_name,
	int key_type);

/*! Get key value from the bus resources matching [device[drv_id, minor_bus], key name, key type] 
 * if no matching key NULL is returned.
 *
 * This is typically used by device drivers to find a particular device resource.
 *
 * \param dev         The device to search resource for.
 * \param key_name    The key name to search for
 * \param key_type    The key type expected.
 * \return            Returns NULL if no value found matching Key Name and Key Type was found for device.
 */
extern union drvmgr_key_value *drvmgr_dev_key_get(
	struct drvmgr_dev *dev,
	char *key_name,
	int key_type);

/*** DRIVER INTERACE USED TO REQUEST INFORMATION/SERVICES FROM BUS DRIVER ***/

/*! Get parent bus */
static __inline__ struct drvmgr_bus *drvmgr_get_parent(struct drvmgr_dev *dev)
{
	if (dev)
		return dev->parent;
	else
		return NULL;
}

/*! Get Driver of device */
static __inline__ struct drvmgr_drv *drvmgr_get_drv(struct drvmgr_dev *dev)
{
	if (dev)
		return dev->drv;
	else
		return NULL;
}

/*! Get Device pointer from Driver and Driver minor number 
 *
 * \param drv         Driver the device is united with.
 * \param minor       Driver minor number assigned to device.
 * \param pdev        Location where the Device point will be stored.
 * \return            Zero on success. -1 on failure, when device was not found in driver 
 *                    device list.
 */
extern int drvmgr_get_dev(
	struct drvmgr_drv *drv,
	int minor,
	struct drvmgr_dev **pdev);

/*! Get Bus frequency in Hertz. Frequency is stored into address of freq_hz.
 *
 * \param dev        The Device to get Bus frequency for.
 * \param options    Bus-type specific options
 * \param freq_hz    Location where Bus Frequency will be stored.
 */
extern int drvmgr_freq_get(
	struct drvmgr_dev *dev,
	int options,
	unsigned int *freq_hz);

/*! Return 0 if dev is not located on the root bus, 1 if on root bus */
extern int drvmgr_on_rootbus(struct drvmgr_dev *dev);

/*! Get device name prefix, this name can be used to register a unique name in the 
 *  filesystem or to get an idea where the device is located.
 *
 * \param dev         The Device to get the device Prefix for.
 * \param dev_prefix  Location where the prefix will be stored.
 */
extern int drvmgr_get_dev_prefix(struct drvmgr_dev *dev, char *dev_prefix);

/*! Register a shared interrupt handler. Since this service is shared among interrupt 
 *  drivers/handlers the handler[arg] must be installed before the interrupt can be
 *  cleared or disabled. The handler is by default disabled after registration.
 *
 *  \param index      Index is used to identify the IRQ number if hardware has multiple IRQ sources. 
 *                    Normally Index is set to 0 to indicated the first and only IRQ source.
 *                    A negative index is interpreted as a absolute bus IRQ number.
 *  \param isr        Interrupt Service Routine.
 *  \param arg        Optional ISR argument.
 */
extern int drvmgr_interrupt_register(
	struct drvmgr_dev *dev,
	int index,
	const char *info,
	drvmgr_isr isr,
	void *arg);

/*! Unregister an interrupt handler. This also disables the interrupt before unregistering
 *  the interrupt handler.
 *  \param index      Index is used to identify the IRQ number if hardware has multiple IRQ sources. 
 *                    Normally Index is set to 0 to indicated the first and only IRQ source.
 *                    A negative index is interpreted as a absolute bus IRQ number.
 *  \param isr        Interrupt Service Routine, previously registered.
 *  \param arg        Optional ISR argument, previously registered.
 */
extern int drvmgr_interrupt_unregister(
	struct drvmgr_dev *dev,
	int index,
	drvmgr_isr isr,
	void *arg);

/*! Clear (ACK) pending interrupt
 *
 *  \param dev        Device to clear interrupt for.
 *  \param index      Index is used to identify the IRQ number if hardware has multiple IRQ sources. 
 *                    Normally Index is set to 0 to indicated the first and only IRQ source.
 *                    A negative index is interpreted as a absolute bus IRQ number.
 *  \param isr        Interrupt Service Routine, previously registered.
 *  \param arg        Optional ISR argument, previously registered.
 */
extern int drvmgr_interrupt_clear(
	struct drvmgr_dev *dev,
	int index);

/*! Force unmasking/enableing an interrupt on the interrupt controller, this is not normally used,
 *  if used the caller has masked/disabled the interrupt just before.
 *
 *  \param dev        Device to clear interrupt for.
 *  \param index      Index is used to identify the IRQ number if hardware has multiple IRQ sources. 
 *                    Normally Index is set to 0 to indicated the first and only IRQ source.
 *                    A negative index is interpreted as a absolute bus IRQ number.
 *  \param isr        Interrupt Service Routine, previously registered.
 *  \param arg        Optional ISR argument, previously registered.
 */
extern int drvmgr_interrupt_unmask(
	struct drvmgr_dev *dev,
	int index);

/*! Force masking/disable an interrupt on the interrupt controller, this is not normally performed
 *  since this will stop all other (shared) ISRs to be disabled until _unmask() is called.
 *
 *  \param dev        Device to mask interrupt for.
 *  \param index      Index is used to identify the IRQ number if hardware has multiple IRQ sources. 
 *                    Normally Index is set to 0 to indicated the first and only IRQ source.
 *                    A negative index is interpreted as a absolute bus IRQ number.
 */
extern int drvmgr_interrupt_mask(
	struct drvmgr_dev *dev,
	int index);

/*! Translate address 
 * 1. From CPU local bus to a remote bus for example a PCI target (from_remote_to_cpu = 0)
 * 2. From remote bus to CPU local bus (from_remote_to_cpu = 1)
 *
 * src_address the address to translate, dst_address is where the translated address is stored.
 *
 * \param dev                  Device to translate addresses for.
 * \param from_remote_to_cpu   Selection tranlation direction.
 * \param src_address          Address to translate
 * \param dst_address          Location where translated address is stored.
 *
 * Returns -1 if unable to translate. If no map is present src_address is translated 1:1 (just copied).
 */
extern int drvmgr_mmap_translate(
	struct drvmgr_dev *dev,
	int from_remote_to_cpu,
	void *src_address,
	void **dst_address);


/*! Get function pointer from Device Driver or Bus Driver.
 *
 *  Returns 0 if function is available
 */
extern int drvmgr_func_get(void *obj, int funcid, void **func);

/*! Lookup function and call it directly with the four optional arguments */
extern int drvmgr_func_call(void *obj, int funcid, void *a, void *b, void *c, void *d);

/* Builds a Function ID.
 *
 * Used to request optional functions by a bus or device driver
 */
#define DRVMGR_FUNCID(major, minor) ((((major) & 0xffff) << 16) | ((minor) & 0xffff))
#define DRVMGR_FUNCID_NONE 0
#define DRVMGR_FUNCID_END DRVMGR_FUNCID(DRVMGR_FUNCID_NONE, 0)

/* Major Function ID */
enum {
	FUNCID_NONE             = 0x0,
	FUNCID_RW               = 0x1, /* Read/Write functions */
};

/* Select Sub-Function Read/Write function by ID */
#define RW_SIZE_1   0x0001    /* Access Size */
#define RW_SIZE_2   0x0002
#define RW_SIZE_4   0x0004
#define RW_SIZE_8   0x0008
#define RW_SIZE_ANY 0x0000

#define RW_DIR_ANY  0x0000   /* Access Direction */
#define RW_READ     0x0000
#define RW_WRITE    0x0010
#define RW_SET      0x0020

#define RW_TYPE_ANY 0x0000  /* Access type */
#define RW_REG      0x0100
#define RW_MEM      0x0200
#define RW_MEMREG   0x0300
#define RW_CFG      0x0400

#define RW_ARG      0x1000 /* Optional Argument */
#define RW_ERR      0x2000 /* Optional Error Handler */

/* Build a Read/Write function ID */
#define DRVMGR_RWFUNC(minor) DRVMGR_FUNCID(FUNCID_RW, minor)

/* Argument to Read/Write functions, the "void *arg" pointer is returned by
 * RW_ARG. If NULL is returned no argument is needed.
 */
struct drvmgr_rw_arg {
	void *arg;
	struct drvmgr_dev *dev;
};

/* Standard Read/Write function types */
typedef uint8_t (*drvmgr_r8)(uint8_t *srcadr);
typedef uint16_t (*drvmgr_r16)(uint16_t *srcadr);
typedef uint32_t (*drvmgr_r32)(uint32_t *srcadr);
typedef uint64_t (*drvmgr_r64)(uint64_t *srcadr);
typedef void (*drvmgr_w8)(uint8_t *dstadr, uint8_t data);
typedef void (*drvmgr_w16)(uint16_t *dstadr, uint16_t data);
typedef void (*drvmgr_w32)(uint32_t *dstadr, uint32_t data);
typedef void (*drvmgr_w64)(uint64_t *dstadr, uint64_t data);
/* READ/COPY a memory area located on bus into CPU memory.
 * From 'src' (remote) to the destination 'dest' (local), n=number of bytes
 */
typedef int (*drvmgr_rmem)(void *dest, const void *src, int n);
/* WRITE/COPY a user buffer located in CPU memory to a location on the bus.
 * From 'src' (local) to the destination 'dest' (remote), n=number of bytes
 */
typedef int (*drvmgr_wmem)(void *dest, const void *src, int n);
/* Set a memory area to the byte value given in c, see LIBC memset(). Memset is
 * implemented by calling wmem() multiple times with a "large" buffer.
 */
typedef int (*drvmgr_memset)(void *dstadr, int c, size_t n);

/* Read/Write function types with additional argument */
typedef uint8_t (*drvmgr_r8_arg)(uint8_t *srcadr, void *a);
typedef uint16_t (*drvmgr_r16_arg)(uint16_t *srcadr, void *a);
typedef uint32_t (*drvmgr_r32_arg)(uint32_t *srcadr, void *a);
typedef uint64_t (*drvmgr_r64_arg)(uint64_t *srcadr, void *a);
typedef void (*drvmgr_w8_arg)(uint8_t *dstadr, uint8_t data, void *a);
typedef void (*drvmgr_w16_arg)(uint16_t *dstadr, uint16_t data, void *a);
typedef void (*drvmgr_w32_arg)(uint32_t *dstadr, uint32_t data, void *a);
typedef void (*drvmgr_w64_arg)(uint64_t *dstadr, uint64_t data, void *a);
typedef int (*drvmgr_rmem_arg)(void *dest, const void *src, int n, void *a);
typedef int (*drvmgr_wmem_arg)(void *dest, const void *src, int n, void *a);
typedef int (*drvmgr_memset_arg)(void *dstadr, int c, size_t n, void *a);

/* Report an error to the parent bus of the device */
typedef void (*drvmgr_rw_err)(struct drvmgr_rw_arg *a, struct drvmgr_bus *bus,
				int funcid, void *adr);

/* Helper function for buses that implement the memset() over wmem() */
extern void drvmgr_rw_memset(
	void *dstadr,
	int c,
	size_t n,
	void *a,
	drvmgr_wmem_arg wmem
	);

/*** PRINT INFORMATION ABOUT DRIVER MANAGER ***/

/*! Calls func() for every device found matching the search requirements of 
 * set_mask and clr_mask. Each bit set in set_mask must be set in the 
 * device state bit mask (dev->state), and Each bit in the clr_mask must
 * be cleared in the device state bit mask (dev->state). There are three
 * special cases:
 *
 * 1. If state_set_mask and state_clr_mask are zero the state bits are
 *    ignored and all cores are treated as a match.
 *
 * 2. If state_set_mask is zero the function func will not be called due to 
 *    a bit being set in the state mask.
 *
 * 3. If state_clr_mask is zero the function func will not be called due to 
 *    a bit being cleared in the state mask.
 *
 * If the function func() returns a non-zero value then for_each_dev will
 * return imediatly with the same return value as func() returned.
 *
 * \param devlist            The list to iterate though searching for devices.
 * \param state_set_mask     Defines the bits that must be set in dev->state
 * \param state_clr_mask     Defines the bits that must be cleared in dev->state
 * \param func               Function called on each 
 *
 */
extern int drvmgr_for_each_dev(
	struct drvmgr_list *devlist,
	unsigned int state_set_mask,
	unsigned int state_clr_mask,
	int (*func)(struct drvmgr_dev *dev, void *arg),
	void *arg);

/*! Print all drivers */
extern void drvmgr_print_drvs(int show_devs);

/* Print all devices */
#define PRINT_DEVS_FAILED	0x01	/* Failed during initialization */
#define PRINT_DEVS_ASSIGNED	0x02	/* Driver assigned */
#define PRINT_DEVS_UNASSIGNED	0x04	/* Driver not assigned */
#define PRINT_DEVS_IGNORED	0x08	/* Device ignored on user's request */
#define PRINT_DEVS_ALL		(PRINT_DEVS_FAILED | \
				PRINT_DEVS_ASSIGNED | \
				PRINT_DEVS_UNASSIGNED |\
				PRINT_DEVS_IGNORED)

/*! Print number of devices, buses and drivers */
extern void drvmgr_summary(void);

/*! Print devices with certain condictions met according to 'options' */
extern void drvmgr_print_devs(unsigned int options);

/*! Print device/bus topology */
extern void drvmgr_print_topo(void);

/*! Print the memory usage
 * Only accounts for data structures. Not for the text size.
 */
extern void drvmgr_print_mem(void);

#define OPTION_DEV_GENINFO   0x00000001
#define OPTION_DEV_BUSINFO   0x00000002
#define OPTION_DEV_DRVINFO   0x00000004
#define OPTION_DRV_DEVS      0x00000100
#define OPTION_BUS_DEVS      0x00010000
#define OPTION_RECURSIVE     0x01000000
#define OPTION_INFO_ALL      0xffffffff

/*! Print information about a driver manager object (device, driver, bus) */
extern void drvmgr_info(void *id, unsigned int options);

/*! Get information about a device */
extern void drvmgr_info_dev(struct drvmgr_dev *dev, unsigned int options);

/*! Get information about a bus */
extern void drvmgr_info_bus(struct drvmgr_bus *bus, unsigned int options);

/*! Get information about a driver */
extern void drvmgr_info_drv(struct drvmgr_drv *drv, unsigned int options);

/*! Get information about all devices on a bus */
extern void drvmgr_info_devs_on_bus(struct drvmgr_bus *bus, unsigned int options);

/*! Get information about all devices in the system (on all buses) */
extern void drvmgr_info_devs(unsigned int options);

/*! Get information about all drivers in the system */
extern void drvmgr_info_drvs(unsigned int options);

/*! Get information about all buses in the system */
extern void drvmgr_info_buses(unsigned int options);

/*! Get Driver by Driver ID */
extern struct drvmgr_drv *drvmgr_drv_by_id(uint64_t id);

/*! Get Driver by Driver Name */
extern struct drvmgr_drv *drvmgr_drv_by_name(const char *name);

#ifdef __cplusplus
}
#endif

#endif
