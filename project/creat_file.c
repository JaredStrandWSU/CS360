//creates a new file given a path 
void creat_file(char path[124])
{
	int i, ino;
	MINODE *pmip;
	INODE *pip;

	char buf[1024];
	char temp1[1024], temp2[1024];
	char parent_name[1024], child_name[1024];

	//create two copies of the pathname
	strcpy(temp1, path);
	strcpy(temp2, path);
	//dirname is parent name, basename is child to be created name
	strcpy(parent_name, dirname(temp1));
	strcpy(child_name, basename(temp2));

	//get the ino of the parent
	ino = getino(running->cwd, parent_name);
	printf("%d\n", ino);
	//set pmip to the parent minode from its ino
	pmip = iget(dev, ino);
	//pip now points at the parents direct inode
	pip = &pmip->INODE;

	if(!pmip)
	{
		printf("ERROR: parent does not exist\n");
		return;
	}

	//check if dir
	if(!S_ISDIR(pip->i_mode))
	{
		printf("ERROR: parent is not directory\n");
		return;
	}

	//check if dir already exists
	if(getino(running->cwd, path) != 0)
	{
		printf("ERROR: %s already exists\n", path);
		return;
	}

	printf("running creat\n");
	//accepts the parent mem pointer and creates a child file called child_name in its dir
	my_creat(pmip, child_name);

	//size = 0, linkcount = 1, don't increment parent's links count
	pip->i_atime = time(0L);

	//Set dirty and iput
	pmip->dirty = 1;

	iput(pmip);

	return;
}

int my_creat(MINODE *pmip, char *child_name)
{
	int i;
	//allocate inode for new file
	int ino = ialloc(dev);

	//create it in memory
	MINODE *mip = iget(dev, ino);
	//ip now points at the newly created inode
	INODE *ip = &mip->INODE;

	//set all the fields for a new file
	ip->i_mode = 0x81A4; //file type
	ip->i_uid  = running->uid; // Owner uid
    ip->i_gid  = running->gid; // Group Id
	//We set the size to 0 because it is an empty file
    ip->i_size = 0;	// Size in bytes
    ip->i_links_count = 1; // Links to parent directory
    ip->i_atime = time(0L); // Set last access to current time
	ip->i_ctime = time(0L);
	ip->i_mtime = time(0L);

	//Set to 0 cause we dont need . and .. cause it isnt a dir
	ip->i_blocks = 0;

	//set data blocks to 0
	for(i = 0; i < 15; i++)
	{
		ip->i_block[i] = 0;
	}

	//set dirty and iput
	mip->dirty = 1;
	iput(mip);

	//enters the files name in the parent directory to complete making new file
	//dynamically adds name to squeeze name into as few blocks as possible
	enter_name(pmip, ino, child_name);//enter name defined in mkdir.c

	//returns ino of new file
	return ino;
}
