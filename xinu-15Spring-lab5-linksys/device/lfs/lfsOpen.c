/* lfsOpen.c  -  lfsOpen */

#include <xinu.h>

/*------------------------------------------------------------------------
 * lfsOpen - open a file and allocate a local file pseudo-device
 *------------------------------------------------------------------------
 */

bool8 strcmp(char *, char *);
void strcpy(char *, char *);

devcall	lfsOpen (
	 struct	dentry	*devptr,	/* entry in device switch table	*/
	 char	*name,			/* name of file to open		*/
	 char	*mode			/* mode chars: 'r' 'w' 'o' 'n'	*/
	)
{
	did32		lfnext;		/* minor number of an unused	*/
	struct	lflcblk	*lfptr;		/* ptr to open file table entry	*/
	int32	mbits;			/* mode bits			*/
	
	char valueAfter[LF_NUM_DIR_ENT][LF_NAME_LEN];
	int depth = 0;
	int32 i = 0;

	if(name[i] == '/' && name[i] == '\0')
	{
		valueAfter[0][0] = '/';
		valueAfter[0][1] = '\0';
		depth = 1;
	}
	else
	{	
		int indexedValues = 0;
		for(;depth < LF_NUM_DIR_ENT;){	
		
			if(name[indexedValues] == '/')
			{
				indexedValues++;
			}
			
			int tokenIndex = 0;
			while(name[indexedValues] != '/' && name[indexedValues] != '\0')
			{
				valueAfter[depth][tokenIndex++] = name[indexedValues++];
			}
			valueAfter[depth][tokenIndex] = '\0';
			depth++;
			
			if(name[indexedValues] == '\0')
				break;

			if(name[indexedValues] == '/' && name[indexedValues+1] == '\0')
				break;
		}
	}
	
	if(depth == 1 && valueAfter[0][0] == '/' && valueAfter[0][1] == '\0')
	{
		//kprintf("Can't open root\r\n");
		return SYSERR;
	}
	
	/* Parse mode argument and convert to binary */

	mbits = lfgetmode(mode);
	if (mbits == SYSERR) {
		return SYSERR;
	}

	/* If named file is already open, return SYSERR */

	lfnext = SYSERR;
	
	for(i = 0; i < Nlfl; i++)
	{
		lfptr = &lfltab[i];
		if(lfptr->lfstate == LF_FREE)
		{
			if(lfnext == SYSERR)
			{
				lfnext = i;
			}
		}
		else
		{
			int j = 0;
			if(lfptr->depth != depth)
			{
				continue;
			}
			
			for(j = 0; j < depth; j++)
			{
				if(!strcmp(lfptr->path[j], valueAfter[j]))
					break;
			}
			if(j == depth)
			{
				//kprintf("File is already open\r\n");
				return SYSERR;
			}
		}
	}

	if (lfnext == SYSERR) {	/* no slave file devices are available	*/
		return SYSERR;
	}

	struct dentry firstDevice;
	struct dentry olderDevice;

	wait(Lf_data.lf_mutex);

	if(Lf_data.lf_dirpresent == FALSE)
	{	
		//kprintf("Initialize rootDir\r\n");
		struct lfdir rootDir;
		if(read(Lf_data.lf_dskdev, (char *)&rootDir, 0) == SYSERR)
		{
			//kprintf("Failed to read root\r\n");
			signal(Lf_data.lf_mutex);
			return SYSERR;
		}
		
		//kprintf("rootDir size = %d\r\n", rootDir.lfd_size);
		Lf_data.lf_dir = rootDir;
		Lf_data.lf_dirpresent = TRUE;
		Lf_data.lf_dirdirty = FALSE;
	}

	signal(Lf_data.lf_mutex);

	//kprintf("rootDir initialized\r\n");

	(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
	(*(lfltab + Nlfl + 1)).lfpos = 0;
	(*(lfltab + Nlfl + 1)).lfinum = LF_INULL;
	(*(lfltab + Nlfl + 1)).lfdnum = LF_DNULL;
	(*(lfltab + Nlfl + 1)).lfbyte = &((*(lfltab + Nlfl + 1))).lfdblock[LF_BLKSIZ];
	(*(lfltab + Nlfl + 1)).lfibdirty = 0;
	(*(lfltab + Nlfl + 1)).lfdbdirty = 0;
	(*(lfltab + Nlfl + 1)).size = -1;
	(*(lfltab + Nlfl + 1)).firstId = LF_INULL;
	memset((char *)(*(lfltab + Nlfl + 1)).path, NULLCH, LF_NUM_DIR_ENT*LF_NAME_LEN);
	(*(lfltab + Nlfl + 1)).depth = -1;

	(*(lfltab + Nlfl)).lfstate = LF_FREE;
	(*(lfltab + Nlfl)).lfpos = 0;
	(*(lfltab + Nlfl)).lfinum = LF_INULL;
	(*(lfltab + Nlfl)).lfdnum = LF_DNULL;
	(*(lfltab + Nlfl)).lfbyte = &((*(lfltab + Nlfl + 1))).lfdblock[LF_BLKSIZ];
	(*(lfltab + Nlfl)).lfibdirty = 0;
	(*(lfltab + Nlfl)).lfdbdirty = 0;
	(*(lfltab + Nlfl)).size = -1;
	(*(lfltab + Nlfl)).firstId = LF_INULL;
	memset((char *)	(*(lfltab + Nlfl)).path, NULLCH, LF_NUM_DIR_ENT*LF_NAME_LEN);
	(*(lfltab + Nlfl)).depth = -1;

	(*(lfltab + Nlfl + 1)).lfstate = LF_USED;
	(*(lfltab + Nlfl + 1)).size = Lf_data.lf_dir.lfd_size;
	(*(lfltab + Nlfl + 1)).firstId = Lf_data.lf_dir.lfd_ilist;

	int currentDepth = 0;

	struct ldentry currentEntry;
	//struct ldentry *entry = &currentEntry;
	firstDevice.dvminor = Nlfl+1;
	olderDevice.dvminor = Nlfl;
	
	//kprintf("Entering loop\r\n");
	//kprintf("sizeof(struct ldentry) = %d\r\n", sizeof(struct ldentry));
	while(currentDepth < depth-1 && lflRead(&firstDevice, (char *)(&currentEntry), sizeof(struct ldentry)) == sizeof(struct ldentry))
	{

		char *first  = currentEntry.ld_name;
		char *second = *(valueAfter + currentDepth);

		if(strcmp(first, second) && currentEntry.isUsed)
		{
			if(currentEntry.type != LF_TYPE_DIR)
			{
				//kprintf("%s is a file, not a dir\r\n", entry->ld_name);
				return SYSERR;
			}
			
			memcpy(	((lfltab + Nlfl)), (((lfltab + Nlfl + 1))), sizeof(struct lflcblk));

			((*(lfltab + Nlfl + 1))).lfstate = LF_FREE;
			((*(lfltab + Nlfl + 1))).lfpos = 0;
			((*(lfltab + Nlfl + 1))).lfinum = LF_INULL;
			((*(lfltab + Nlfl + 1))).lfdnum = LF_DNULL;
			((*(lfltab + Nlfl + 1))).lfbyte = &	((*(lfltab + Nlfl + 1))).lfdblock[LF_BLKSIZ];
			((*(lfltab + Nlfl + 1))).lfibdirty = 0;
			((*(lfltab + Nlfl + 1))).lfdbdirty = 0;
			((*(lfltab + Nlfl + 1))).size = -1;
			((*(lfltab + Nlfl + 1))).firstId = LF_INULL;
			memset((char *)	((*(lfltab + Nlfl + 1))).path, NULLCH, LF_NUM_DIR_ENT*LF_NAME_LEN);
			((*(lfltab + Nlfl + 1))).depth = -1;

			((*(lfltab + Nlfl + 1))).lfstate = LF_USED;
			((*(lfltab + Nlfl + 1))).size = currentEntry.ld_size;
			((*(lfltab + Nlfl + 1))).firstId = currentEntry.ld_ilist;
			currentDepth++;
		}
	}

	if(depth-1 != currentDepth)
	{
		//kprintf("moveToDir failed\r\n");
		return SYSERR;
	}

	struct ldentry fileInfo;

	firstDevice.dvminor = Nlfl+1;
	olderDevice.dvminor = Nlfl;

	uint32 changingPosition = 0;
	bool8 positionOrNot = 0;
	bool8 fnf = 1;

	while(1){
			int sizeOfEntry = sizeof(struct ldentry);	
			int lfRead = lflRead(&firstDevice, (char *)(&fileInfo), sizeof(struct ldentry));
			if(lfRead == sizeOfEntry){
				if(!fileInfo.isUsed){
					if(!positionOrNot){
							changingPosition = 	(*(lfltab + Nlfl + 1)).lfpos - sizeof(struct ldentry);
							positionOrNot = 1;
						}
						continue;
			}

			char *first  = fileInfo.ld_name;
			char *second = *(valueAfter + (depth-1));
			if(strcmp(first, second) && fileInfo.isUsed){
				if(fileInfo.type == LF_TYPE_DIR){
					(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
					(*(lfltab + Nlfl)).lfstate = LF_FREE;
					return SYSERR;
				}

				if(mbits & LF_MODE_N){
					(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
					(*(lfltab + Nlfl)).lfstate = LF_FREE;
					return SYSERR;
				}

					(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
					(*(lfltab + Nlfl)).lfstate = LF_FREE;
					fnf = 0;
				}
			}
			else{
				break;			
			}
	}

	if(fnf)
	{
		//kprintf("File not found\r\n");
		if(mbits & LF_MODE_O)
		{
			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
			(*(lfltab + Nlfl)).lfstate = LF_FREE;
			return SYSERR;
		}

		if(positionOrNot)
		{
			if(lflSeek(&firstDevice, changingPosition) == SYSERR)
			{
				//kprintf("Seek failed\r\n");
			}
		}
		
		//kprintf("Creating a new file\r\n");
		fileInfo.ld_size = 0;
		fileInfo.ld_ilist = LF_INULL;
		fileInfo.type = LF_TYPE_FILE;
		fileInfo.isUsed = (bool8)1;
		strcpy(fileInfo.ld_name, valueAfter[depth-1]);

		if(lflWrite(&firstDevice, (char *)(&fileInfo), sizeof(struct ldentry)) == SYSERR)
		{
			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
			(*(lfltab + Nlfl)).lfstate = LF_FREE;
			return SYSERR;
		}

		if(lfflush(	((lfltab + Nlfl + 1))) == SYSERR)
		{
			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
			(*(lfltab + Nlfl)).lfstate = LF_FREE;
			return SYSERR;
		}

		if(positionOrNot)
		{
			lfptr = &lfltab[lfnext];
			lfptr->lfstate = LF_USED;
			lfptr->lfmode = mbits & LF_MODE_RW;

			/* File starts at position 0 */

			lfptr->lfpos     = 0;

			/* Neither index block nor data block are initially valid	*/

			lfptr->lfinum    = LF_INULL;
			lfptr->lfdnum    = LF_DNULL;

			/* Initialize byte pointer to address beyond the end of the	*/
			/*	buffer (i.e., invalid pointer triggers setup)		*/

			lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];
			lfptr->lfibdirty = 0;
			lfptr->lfdbdirty = 0;

			lfptr->size = fileInfo.ld_size;
			lfptr->firstId = fileInfo.ld_ilist;
			memcpy(lfptr->path,valueAfter,LF_NAME_LEN * LF_NUM_DIR_ENT);
			lfptr->depth = depth;

			return lfptr->lfdev;
		}
		else if((*(lfltab + Nlfl)).lfstate == LF_FREE)
		{
			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
			wait(Lf_data.lf_mutex);
			Lf_data.lf_dir.lfd_size += sizeof(struct ldentry);
			Lf_data.lf_dirdirty = 1;
			signal(Lf_data.lf_mutex);

			lfptr = &lfltab[lfnext];
			lfptr->lfstate = LF_USED;
			lfptr->lfmode = mbits & LF_MODE_RW;

			/* File starts at position 0 */

			lfptr->lfpos     = 0;

			/* Neither index block nor data block are initially valid	*/

			lfptr->lfinum    = LF_INULL;
			lfptr->lfdnum    = LF_DNULL;

			/* Initialize byte pointer to address beyond the end of the	*/
			/*	buffer (i.e., invalid pointer triggers setup)		*/

			lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];
			lfptr->lfibdirty = 0;
			lfptr->lfdbdirty = 0;

			lfptr->size = fileInfo.ld_size;
			lfptr->firstId = fileInfo.ld_ilist;
			memcpy(lfptr->path,valueAfter,LF_NAME_LEN * LF_NUM_DIR_ENT);
			lfptr->depth = depth;

			return lfptr->lfdev;
		}

		struct ldentry grandParentDirEntry;

		lflSeek(&olderDevice, (*(lfltab + Nlfl)).lfpos - sizeof(struct ldentry));

		if(lflRead(&olderDevice, (char *)&grandParentDirEntry, sizeof(struct ldentry)) == SYSERR)
		{
			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
			(*(lfltab + Nlfl)).lfstate = LF_FREE;
			return SYSERR;
		}

		grandParentDirEntry.ld_size += sizeof(struct ldentry);
		grandParentDirEntry.ld_ilist = 	(*(lfltab + Nlfl + 1)).firstId;

		lflSeek(&olderDevice, (*(lfltab + Nlfl)).lfpos - sizeof(struct ldentry));

		if(lflWrite(&olderDevice,(char*)&grandParentDirEntry,sizeof(struct ldentry)) == SYSERR)
		{

			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
			(*(lfltab + Nlfl)).lfstate = LF_FREE;
			return SYSERR;
		}

		if(lfflush(	((lfltab + Nlfl))) == SYSERR)
		{
			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
			(*(lfltab + Nlfl)).lfstate = LF_FREE;
			
			return SYSERR;
		}
		
			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
			(*(lfltab + Nlfl)).lfstate = LF_FREE;

			lfptr = &lfltab[lfnext];
			lfptr->lfstate = LF_USED;
			lfptr->lfmode = mbits & LF_MODE_RW;

			/* File starts at position 0 */

			lfptr->lfpos     = 0;

			/* Neither index block nor data block are initially valid	*/

			lfptr->lfinum    = LF_INULL;
			lfptr->lfdnum    = LF_DNULL;

			/* Initialize byte pointer to address beyond the end of the	*/
			/*	buffer (i.e., invalid pointer triggers setup)		*/

			lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];
			lfptr->lfibdirty = FALSE;
			lfptr->lfdbdirty = FALSE;

			lfptr->size = fileInfo.ld_size;
			lfptr->firstId = fileInfo.ld_ilist;
			memcpy(lfptr->path,valueAfter,LF_NAME_LEN * LF_NUM_DIR_ENT);
			lfptr->depth = depth;

			return lfptr->lfdev;

	}



	/* Initialize the local file pseudo-device */

	lfptr = &lfltab[lfnext];
	lfptr->lfstate = LF_USED;
	lfptr->lfmode = mbits & LF_MODE_RW;

	/* File starts at position 0 */

	lfptr->lfpos     = 0;

	/* Neither index block nor data block are initially valid	*/

	lfptr->lfinum    = LF_INULL;
	lfptr->lfdnum    = LF_DNULL;

	/* Initialize byte pointer to address beyond the end of the	*/
	/*	buffer (i.e., invalid pointer triggers setup)		*/

	lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];
	lfptr->lfibdirty = FALSE;
	lfptr->lfdbdirty = FALSE;

	lfptr->size = fileInfo.ld_size;
	lfptr->firstId = fileInfo.ld_ilist;
	memcpy(lfptr->path,valueAfter,LF_NAME_LEN * LF_NUM_DIR_ENT);
	lfptr->depth = depth;


	return lfptr->lfdev;
}

bool8 strcmp(char *str1, char *str2)
{
	while(*str1 != '\0' && *str1 == *str2)
	{
		str1++;
		str2++;
	}

	return (*str1 == *str2) && (*str1 == '\0');
}

void strcpy(char *str1, char *str2)
{
	while(*str2 != '\0')
	{
		*str1 = *str2;
		str1++;
		str2++;
	}

	*str1 = '\0';
}
