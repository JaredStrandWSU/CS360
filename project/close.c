
//Takes in a file descriptor to close
void close_file(int fd)
{
	//allocate an oft pointer
	int i;
	OFT *fp;

	//Checks if the fd is in range to be accessed
	if(fd < 0 || fd >= NFD)
	{
		printf("ERROR: fd out of range\n");
		return;
	}

	//Checks if the fd is on the OpenFileTable
	//Cause if it isn't, this fd must not be open
	//for all oft
	for(i = 0; i < NOFT; i++)
	{
		//oft* fp is set to point directly at the oft struct
		fp = &OpenFileTable[i];

		//breaks if it finds the fd on the OpenFileTable
		if(fp->inodeptr == running->fd[fd]->inodeptr)
			break;

		if(i == NOFT - 1)
		{
			printf("ERROR: File not in OpenFileTable\n");
			return;
		}
	}

	//sets fp, an OFT, to the correct fd from the running proccess
	//does this instead of having it pointing through the openfiltable array
	fp = running->fd[fd];
	running->fd[fd] = NULL;

	//Ensures that the minodes are running as should be
	fp->refCount--;

	//if noone else is using this open file then we can iput the inode of it
	if(fp->refCount == 0)
		iput(fp->inodeptr);

	return;
}

//Calls close file for the given fd
void my_close(char *path)
{
	int fd;

	//check for missing path
	if(!path)
	{
		printf("ERROR: missing path\n");
		return;
	}

	//converts path to fd
	fd = atoi(path);
	close_file(fd);
	return;
}
