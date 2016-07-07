// IK ONKAR

#include <xinu.h>

bool8 strcmp(char *str1, char *str2);
void strcpy(char *str1, char *str2);
int32 split(char *directorypath,  char  valueAfterSplit[LF_NUM_DIR_ENT][LF_NAME_LEN]);

int makeDirectory(char *directorypath)
{
	char valueAfterSplit[LF_NUM_DIR_ENT][LF_NAME_LEN];
	int depth = split(directorypath, valueAfterSplit);

	struct dentry newFirstDevice;
	struct dentry newPreviousDevice;

	wait(Lf_data.lf_mutex);

	if(Lf_data.lf_dirpresent == 0)
	{	

		struct lfdir rootDir;
		if(!(read(Lf_data.lf_dskdev, (char *)&rootDir, 0)) == OK)
		{
			signal(Lf_data.lf_mutex);
			return SYSERR;
		}

		Lf_data.lf_dir = rootDir;
		Lf_data.lf_dirpresent = 1;
		Lf_data.lf_dirdirty = 0;
	}

	signal(Lf_data.lf_mutex);


	(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
	(*(lfltab + Nlfl + 1)).lfpos = 0;
	(*(lfltab + Nlfl + 1)).lfinum = LF_INULL;
	(*(lfltab + Nlfl + 1)).lfdnum = LF_DNULL;
	(*(lfltab + Nlfl + 1)).lfbyte = &((*(lfltab + Nlfl + 1)).lfdblock[LF_BLKSIZ]);

	(*(lfltab + Nlfl + 1)).lfibdirty = 0;
	(*(lfltab + Nlfl + 1)).lfdbdirty = 0;
	(*(lfltab + Nlfl + 1)).size = -1;
	(*(lfltab + Nlfl + 1)).firstId = LF_INULL;
	memset((char *)	(*(lfltab + Nlfl + 1)).path, '\0', LF_NUM_DIR_ENT*LF_NAME_LEN);
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
	memset((char *)	(*(lfltab + Nlfl)).path, '\0', LF_NUM_DIR_ENT*LF_NAME_LEN);
	(*(lfltab + Nlfl)).depth = -1;

	(*(lfltab + Nlfl + 1)).lfstate = LF_USED;
	(*(lfltab + Nlfl + 1)).size = Lf_data.lf_dir.lfd_size;
	(*(lfltab + Nlfl + 1)).firstId = Lf_data.lf_dir.lfd_ilist;

	int currentDepth = 0;

	struct ldentry currentEntry;


	newFirstDevice.dvminor = Nlfl+1;
	newPreviousDevice.dvminor = Nlfl;

	
	while(1){
		int reading = lflRead(&newFirstDevice, (char *)(&currentEntry), sizeof(struct ldentry));
		int eSize  = sizeof(struct ldentry);
		
		if(currentDepth < depth -1 && reading == eSize){
			if(strcmp(currentEntry.ld_name, valueAfterSplit[currentDepth]) && currentEntry.isUsed)
			{
				if(currentEntry.type != LF_TYPE_DIR)
				{
					return SYSERR;
				}
			
				memcpy(((lfltab + Nlfl)), ((lfltab + Nlfl + 1)), sizeof(struct lflcblk));

				(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
				(*(lfltab + Nlfl + 1)).lfpos = 0;
				(*(lfltab + Nlfl + 1)).lfinum = LF_INULL;
				(*(lfltab + Nlfl + 1)).lfdnum = LF_DNULL;
				(*(lfltab + Nlfl + 1)).lfbyte = &((*(lfltab + Nlfl + 1))).lfdblock[LF_BLKSIZ];
				(*(lfltab + Nlfl + 1)).lfibdirty = 0;
				(*(lfltab + Nlfl + 1)).lfdbdirty = 0;
				(*(lfltab + Nlfl + 1)).size = -1;
				(*(lfltab + Nlfl + 1)).firstId = LF_INULL;
				memset((char *)	(*(lfltab + Nlfl + 1)).path, '\0', LF_NUM_DIR_ENT*LF_NAME_LEN);
				(*(lfltab + Nlfl + 1)).depth = -1;

				(*(lfltab + Nlfl + 1)).lfstate = LF_USED;
				(*(lfltab + Nlfl + 1)).size = currentEntry.ld_size;
				(*(lfltab + Nlfl + 1)).firstId = currentEntry.ld_ilist;
				currentDepth++;
			}
		}
		else{
			break;	
		}
	}



	if(depth-1 != currentDepth)
	{
		return SYSERR;
	}

	uint32 positionToReplace = 0;
	bool8 positionCheck = 0;
	newFirstDevice.dvminor = Nlfl+1;
	newPreviousDevice.dvminor = Nlfl;



	while(1){
		int whatToRead = lflRead(&newFirstDevice, (char *)(&currentEntry), sizeof(struct ldentry));
		int entrySize  = sizeof(struct ldentry);

		if(whatToRead == entrySize){
			if(!currentEntry.isUsed)
			{
				if(!positionCheck)
				{
					positionToReplace = (*(lfltab + Nlfl + 1)).lfpos - sizeof(struct ldentry);
					positionCheck = 1;
				}
				continue;
			}

			char *first  = currentEntry.ld_name;
			char *second = *(valueAfterSplit + (depth-1));
			if(strcmp(first, second) && currentEntry.isUsed)
			{
				if(currentEntry.type == LF_TYPE_DIR)
				{
					(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
					(*(lfltab + Nlfl)).lfstate = LF_FREE;
					return SYSERR;
				}
			}
		}
		else{
			break;	
		}
	}

	if(positionCheck)
	{
		if(lflSeek(&newFirstDevice, positionToReplace) == SYSERR)
		{
			//kprintf("Seek failed\r\n");
		}
	}

    currentEntry.ld_size = 0;
	currentEntry.ld_ilist = LF_INULL;
	currentEntry.type = LF_TYPE_DIR;
	currentEntry.isUsed = (bool8)1;

	char *cpfirst  = currentEntry.ld_name;
	char *cpsecond = *(valueAfterSplit + (depth-1));
    strcpy(cpfirst,cpsecond);

	int nEntrySize = sizeof(struct ldentry);

    if(!(lflWrite(&newFirstDevice, (char *)(&currentEntry), nEntrySize)) == OK)
	{	
   			(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
        	(*(lfltab + Nlfl)).lfstate = LF_FREE;
         	return OK;
    }

    if(!(lfflush(((lfltab + Nlfl + 1)))) == OK)
	{
		(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
		(*(lfltab + Nlfl)).lfstate = LF_FREE;
		return OK;
	}

	if(positionCheck)
	{
		(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
		(*(lfltab + Nlfl)).lfstate = LF_FREE;
		return OK;
	}

	if((*(lfltab + Nlfl)).lfstate == LF_FREE)
	{
		(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
		wait(Lf_data.lf_mutex);
		Lf_data.lf_dir.lfd_size += nEntrySize;
		Lf_data.lf_dirdirty = 1;
		signal(Lf_data.lf_mutex);
		return OK;
	}

	struct ldentry oldestEntry;

	lflSeek(&newPreviousDevice, (*(lfltab + Nlfl)).lfpos - nEntrySize);

	if(lflRead(&newPreviousDevice, (char *)&oldestEntry, nEntrySize) == SYSERR)
	{
		(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
		(*(lfltab + Nlfl)).lfstate = LF_FREE;
		return SYSERR;
	}

	oldestEntry.ld_size += nEntrySize;
	oldestEntry.ld_ilist = 	(*(lfltab + Nlfl + 1)).firstId;

	lflSeek(&newPreviousDevice, (*(lfltab + Nlfl)).lfpos - sizeof(struct ldentry));

	if(lflWrite(&newPreviousDevice,(char*)&oldestEntry,sizeof(struct ldentry)) == SYSERR)
	{

		(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
		(*(lfltab + Nlfl)).lfstate = LF_FREE;
		return SYSERR;
	}

	if(lfflush(((lfltab + Nlfl))) == SYSERR)
	{
		(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
		(*(lfltab + Nlfl)).lfstate = LF_FREE;
			
		return SYSERR;
	}

	(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
	(*(lfltab + Nlfl)).lfstate = LF_FREE;

	return OK;
}


int32 split(char *directorypath,  char  valueAfterSplit[LF_NUM_DIR_ENT][LF_NAME_LEN]){

	
	int depth = 0;
	int32 i = 0;

	if(directorypath[i] == '/' && directorypath[i+1] == '\0')
	{
		valueAfterSplit[0][0] = '/';
		valueAfterSplit[0][1] = '\0';
		depth = 1;
	}
	else
	{	
		int nameIndex = 0;
		for(;depth < LF_NUM_DIR_ENT;)	
		{
			if(directorypath[nameIndex] == '/')
			{
				nameIndex++;
			}
			
			int tokenIndex = 0;
			for(;directorypath[nameIndex] != '/' && directorypath[nameIndex] != '\0';)
			{
				valueAfterSplit[depth][tokenIndex] = directorypath[nameIndex];
				tokenIndex = tokenIndex + 1;
				nameIndex = nameIndex + 1;
			}
			valueAfterSplit[depth][tokenIndex] = '\0';
			depth++;
			
			if(directorypath[nameIndex] == '\0')
				break;

			if(directorypath[nameIndex] == '/' && directorypath[nameIndex+1] == '\0')
				break;
		}
	}
	return depth;
}


