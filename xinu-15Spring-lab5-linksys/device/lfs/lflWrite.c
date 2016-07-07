/* lflWrite.c  -  lfWrite */

#include <xinu.h>

/*------------------------------------------------------------------------
 * lflWrite  --  write data to a previously opened local disk file
 *------------------------------------------------------------------------
 */
devcall	lflWrite (
	  struct dentry *devptr,	/* entry in device switch table */
	  char	*buff,			/* buffer holding data to write	*/
	  int32	count			/* number of bytes to write	*/
	)
{
	int32		i;		/* number of bytes written	*/
	
	//kprintf("In lflWrite\r\n");
	if (count < 0) {
		//kprintf("Exiting lflWrite\r\n");
		return SYSERR;
	}
	for (i=0; i<count; i++) {
		if (lflPutc(devptr, *buff++) == SYSERR) {
			//kprintf("Exiting lflWrite\r\n");
			return SYSERR;
		}
	}
	//kprintf("Exiting lflWrite\r\n");
	return count;
}
