/*! Structure hold all information the driver manager needs to know of. Used internally
 *  by Driver Manager routines.
 */
struct rtems_driver_manager {
	int	level;
	int	initializing_objs;

	/* The first device - The root device and it's driver */
	struct rtems_drvmgr_dev_info	*root_dev;
	struct rtems_drvmgr_drv_info	*root_drv;

	/*!< Linked list of all registered drivers */
	struct rtems_drvmgr_list	drivers;

	/* Buses that reached a certain initialization level.
	 * Lists by Level:
	 *  N=0         - Not intialized, just registered
	 *  N=1..MAX-1  - Reached init level N
	 *  N=MAX       - Successfully initialized bus
	 */
	struct rtems_drvmgr_list	buses[DRVMGR_LEVEL_MAX+1];
	/* Buses failed to initialize or has been removed by not freed */
	struct rtems_drvmgr_list	buses_inactive;		

	/* Devices that reached a certain initialization level.
	 * Lists by Level:
	 *  N=0         - Not intialized, just registered
	 *  N=1..MAX-1  - Reached init level N
	 *  N=MAX       - Successfully initialized device
	 */
	struct rtems_drvmgr_list	devices[DRVMGR_LEVEL_MAX+1];	
	/*!< Devices failed to initialize, removed, ignored, no driver */
	struct rtems_drvmgr_list	devices_inactive;
};

extern struct rtems_driver_manager drv_mgr;
