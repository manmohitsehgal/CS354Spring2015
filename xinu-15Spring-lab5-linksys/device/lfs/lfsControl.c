#include <xinu.h>

devcall lfsControl(struct dentry *devptr, int32 func, int32 arg1, int32 arg2)
{
	int retval;
	switch(func)
	{
		case LF_CTL_MKDIR:
			{
				retval = makeDirectory((char *)arg1);
				return retval;
			}
		default:
			return SYSERR;
	}
	
	return OK;
}
