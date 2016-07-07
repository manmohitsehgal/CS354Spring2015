//IK ONKAR
#include <xinu.h>

int32 splitAgain(char *directorypath,  char  tokens[FILE_DEPTH][LF_NAME_LEN]);
bool8 strcmp(char *str1, char *str2);
void strcpy(char *str1, char *str2);

int lflistdirh(did32 diskdev, char *directorypath)
{
	
	char splitedValue[FILE_DEPTH][LF_NAME_LEN];

	int depth = splitAgain(directorypath, splitedValue);
	int fileDepth = 0;


	if(splitedValue[0][0] == '/' && splitedValue[0][1] == '\0'){
		depth = 0;
	}
	fileDepth = depth;

	struct dentry pointerToDevice, parentDevicePointer;

	wait(Lf_data.lf_mutex);
	if(Lf_data.lf_dirpresent == 0)
	{

		struct lfdir directoryInformation;
		
		int whatToRead = read(Lf_data.lf_dskdev,(char*)&directoryInformation,ROOT);

		if(whatToRead == SYSERR)
		{
			signal(Lf_data.lf_mutex);
			return SYSERR;
		}
		Lf_data.lf_dir = directoryInformation;
		Lf_data.lf_dirpresent = 1;
		Lf_data.lf_dirdirty = 0;
	}
	signal(Lf_data.lf_mutex);
	pointerToDevice.dvminor=Nlfl+1;
	parentDevicePointer.dvminor=Nlfl;

	(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
	(*(lfltab + Nlfl + 1)).lfpos = 0;
	(*(lfltab + Nlfl + 1)).lfinum = LF_INULL;
	(*(lfltab + Nlfl + 1)).lfdnum = LF_DNULL;
	(*(lfltab + Nlfl + 1)).lfbyte = &	(*(lfltab + Nlfl + 1)).lfdblock[LF_BLKSIZ];
	(*(lfltab + Nlfl + 1)).lfibdirty = 0;
	(*(lfltab + Nlfl + 1)).lfdbdirty = 0;
	(*(lfltab + Nlfl + 1)).size = -1;
	(*(lfltab + Nlfl + 1)).firstId = LF_INULL;
	memset((char*)	(*(lfltab + Nlfl + 1)).path,'\0',FILE_DEPTH*LF_NAME_LEN);
	(*(lfltab + Nlfl + 1)).depth = -1;

	(*(lfltab + Nlfl)).lfstate = LF_FREE;
	(*(lfltab + Nlfl)).lfpos = 0;
	(*(lfltab + Nlfl)).lfinum = LF_INULL;
	(*(lfltab + Nlfl)).lfdnum = LF_DNULL;
	(*(lfltab + Nlfl)).lfbyte = &(*(lfltab + Nlfl)).lfdblock[LF_BLKSIZ];
	(*(lfltab + Nlfl)).lfibdirty = 0;
	(*(lfltab + Nlfl)).lfdbdirty = 0;
	(*(lfltab + Nlfl)).size = -1;
	(*(lfltab + Nlfl)).firstId = LF_INULL;
	memset((char*)(*(lfltab + Nlfl)).path,'\0',FILE_DEPTH*LF_NAME_LEN);
	(*(lfltab + Nlfl)).depth = -1;

	(*(lfltab + Nlfl + 1)).lfstate = LF_USED;
	(*(lfltab + Nlfl + 1)).size = Lf_data.lf_dir.lfd_size;
	(*(lfltab + Nlfl + 1)).firstId = Lf_data.lf_dir.lfd_ilist;
	int d_i =0; 

	struct ldentry currentDirEntry;

	while(1){
	
		if(d_i < fileDepth)
		{	
			int lfRead = lflRead(&pointerToDevice,(char*)(&currentDirEntry),sizeof(struct ldentry));
			int sizeOfEntry = sizeof(struct ldentry);

			if(lfRead == sizeOfEntry){
				char * one = currentDirEntry.ld_name;
				char * two = splitedValue[d_i];
				if(strcmp(one,two)&& currentDirEntry.isUsed)
				{
					if(currentDirEntry.type != LF_TYPE_DIR)
					{		
						return SYSERR;
					}

					memcpy(	((lfltab + Nlfl )),((lfltab + Nlfl + 1)),sizeof(struct lflcblk));

					(*(lfltab + Nlfl + 1)).lfstate = LF_FREE;
					(*(lfltab + Nlfl + 1)).lfpos = 0;
					(*(lfltab + Nlfl + 1)).lfinum = LF_INULL;
					(*(lfltab + Nlfl + 1)).lfdnum = LF_DNULL;
					(*(lfltab + Nlfl + 1)).lfbyte = &(*(lfltab + Nlfl + 1)).lfdblock[LF_BLKSIZ];
					(*(lfltab + Nlfl + 1)).lfibdirty = 0;
					(*(lfltab + Nlfl + 1)).lfdbdirty = 0;
					(*(lfltab + Nlfl + 1)).size = -1;
					(*(lfltab + Nlfl + 1)).firstId = LF_INULL;
									memset((char*)	(*(lfltab + Nlfl + 1)).path,'\0',FILE_DEPTH*LF_NAME_LEN);
					(*(lfltab + Nlfl + 1)).depth = -1;

					(*(lfltab + Nlfl + 1)).lfstate = LF_USED;
					(*(lfltab + Nlfl + 1)).size = currentDirEntry.ld_size;
					(*(lfltab + Nlfl + 1)).firstId = currentDirEntry.ld_ilist;
					++d_i;
				}
			}
			else{
				break;			
			}
		}
		else{
			break;		
		}
	}
	int totalDepth = 0;
	if(!(fileDepth != d_i))
	{
		totalDepth = OK;
	}
	else{
		totalDepth = SYSERR;				
	}
	

	if(totalDepth == SYSERR){
	
		return SYSERR;
	}
	struct dentry devEnt;
	devEnt = devtab[diskdev];

	struct ldentry ldEnt;

	devEnt.dvminor = Nlfl+1;
	
	int32 entrySize  = sizeof(struct ldentry);
	

	while(1){
		int currentSize = lflRead(&devEnt, (char*) &ldEnt, sizeof(struct ldentry));
		if(currentSize == entrySize){
			if(depth != 0){
				kprintf("%s/%s\r\n",directorypath , ldEnt.ld_name);
			}
			else{
				kprintf("/%s\r\n", ldEnt.ld_name);
			}		
		}
		else{
			break;
		} 	
	}

	return OK;
	
	
}

int32 splitAgain(char *directorypath,  char  tokens[FILE_DEPTH][LF_NAME_LEN]){

	
	int depth = 0;
	int32 i = 0;
	int total = 0;

	if(directorypath[i] == '/' && directorypath[i+1] == '\0')
	{
		tokens[0][0] = '/';
		tokens[0][1] = '\0';
		depth = 1;
		total = total + 1;
	}
	else
	{	
		int nameIndex = 0;
		for(;depth < LF_NUM_DIR_ENT;)	
		{
			total = total + 1;
			if(directorypath[nameIndex] == '/')
			{
				nameIndex++;
			}
			
			int tokenIndex = 0;
			for(;directorypath[nameIndex] != '/' && directorypath[nameIndex] != '\0';)
			{
				tokens[depth][tokenIndex] = directorypath[nameIndex];
				tokenIndex = tokenIndex + 1;
				nameIndex = nameIndex + 1;
			}
			tokens[depth][tokenIndex] = '\0';
			depth++;
			
			if(directorypath[nameIndex] == '\0')
				total = 0;
				break;

			if(directorypath[nameIndex] == '/' && directorypath[nameIndex+1] == '\0')
				break;
		}
	}
	return depth;
}



