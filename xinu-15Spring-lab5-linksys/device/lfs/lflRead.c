/* lflRead.c  -  lfRead */

#include <xinu.h>

/*------------------------------------------------------------------------
 * lflRead  --  read from a previously opened local file
 *------------------------------------------------------------------------
 */
devcall	lflRead (
	  struct dentry *devptr,	/* entry in device switch table */
	  char	*buff,			/* buffer to hold bytes		*/
	  int32	count			/* max bytes to read		*/
	)
{
	uint32	numread;		/* number of bytes read		*/
//	int32	nxtbyte;		/* character or SYSERR/EOF	*/
	
	//kprintf("In lflRead\r\n");
	if (count < 0) {
		//kprintf("Exiting lflRead: count < 0\r\n");
		return SYSERR;
	}

	/*for (numread=0 ; numread < count ; numread++) {
		nxtbyte = lflGetc(devptr);
		if (nxtbyte == SYSERR) {
			return SYSERR;
		} else if (nxtbyte == EOF) {	
		    if (numread == 0) {
			return EOF;
		    } else {
			return numread;
		    }
		} else {
			*buff++ = (char) (0xff & nxtbyte);
		}
	}*/
	
	struct lflcblk *lfptr = &lfltab[devptr->dvminor];
	
	if(lfptr->lfstate != LF_USED)
	{
		signal(lfptr->lfmutex);
		//kprintf("Exiting lflRead: lfstate != LF_USED\r\n");
		return SYSERR;
	}
	
	if(lfptr->lfpos >= lfptr->size)
	{
		signal(lfptr->lfmutex);
		//kprintf("Exiting lflRead\r\n");
		return EOF;
	}
	
	numread = 0;
	do
	{
		if(lfptr->lfbyte >= &lfptr->lfdblock[LF_BLKSIZ])
		{
			lfsetup(lfptr);
		}
		*buff++ = (char)(0XFF & *lfptr->lfbyte++);
		lfptr->lfpos++;
	}while(++numread < count && lfptr->lfpos < lfptr->size);

	//kprintf("Exiting lflRead\r\n");
	return numread;
}
