#include <drvmgr/drvmgr.h>
#include <drvmgr/drvmgr_list.h>

int drvmgr_for_each_dev(
	struct drvmgr_list *devlist,
	unsigned int state_set_mask,
	unsigned int state_clr_mask,
	int (*func)(struct drvmgr_dev *dev, void *arg),
	void *arg
	)
{
	struct drvmgr_dev *dev;
	int ret;

	/* Get First Device */
	dev = DEV_LIST_HEAD(devlist);
	while ( dev ) {
		if ( ((state_set_mask != 0) && ((dev->state & state_set_mask) == state_set_mask)) ||
		     ((state_clr_mask != 0) && ((dev->state & state_clr_mask) == 0)) ||
		     ((state_set_mask == 0) && (state_clr_mask == 0)) )
			if ( (ret=func(dev, arg)) != 0 )
				return ret;
		dev = dev->next;
	}
	return 0;
}
