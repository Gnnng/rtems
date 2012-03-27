/*  Private driver manager declarations
 *
 *  COPYRIGHT (c) 2009-2011
 *  Aeroflex Gaisler AB
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  Structure hold all information the driver manager needs to know of. Used
 *  internally by Driver Manager routines.
 */

struct rtems_driver_manager {
	int	level;
	int	initializing_objs;

	/* Device tree Lock */
	rtems_id		lock;

	/* The first device - The root device and it's driver */
	struct drvmgr_dev	*root_dev;
	struct drvmgr_drv	*root_drv;

	/*!< Linked list of all registered drivers */
	struct drvmgr_list	drivers;

	/* Buses that reached a certain initialization level.
	 * Lists by Level:
	 *  N=0         - Not intialized, just registered
	 *  N=1..MAX-1  - Reached init level N
	 *  N=MAX       - Successfully initialized bus
	 */
	struct drvmgr_list	buses[DRVMGR_LEVEL_MAX+1];
	/* Buses failed to initialize or has been removed by not freed */
	struct drvmgr_list	buses_inactive;

	/* Devices that reached a certain initialization level.
	 * Lists by Level:
	 *  N=0         - Not intialized, just registered
	 *  N=1..MAX-1  - Reached init level N
	 *  N=MAX       - Successfully initialized device
	 */
	struct drvmgr_list	devices[DRVMGR_LEVEL_MAX+1];
	/*!< Devices failed to initialize, removed, ignored, no driver */
	struct drvmgr_list	devices_inactive;
};

extern struct rtems_driver_manager drv_mgr;

extern void _DRV_Manager_Lock(void);
extern void _DRV_Manager_Unlock(void);
extern int _DRV_Manager_Init_Lock(void);

/* The best solution is to implement the locking with a RW lock, however there
 * is no such API available. Care must be taken so that dead-lock isn't created
 * for example in recursive functions.
 */
#define DRVMGR_LOCK_INIT() _DRV_Manager_Init_Lock()
#define DRVMGR_LOCK_WRITE() _DRV_Manager_Lock()
#define DRVMGR_LOCK_READ() _DRV_Manager_Lock()
#define DRVMGR_UNLOCK() _DRV_Manager_Unlock()
