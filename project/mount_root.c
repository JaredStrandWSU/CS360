//this function mounts the root inode on the device to support the file system
/*
MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MOUNT  mounttab[5];

char names[64][128],*name[64];
int fd, dev, n;
int nblocks, ninodes, bmap, imap, inode_start, iblock;
int inodeBeginBlock;
char pathname[256], parameter[256];
int DEBUG;
*/

//2. Initialize data structures of LEVEL-1
init()
{
	//2 PROCs, P0 with uid=0, P1 with uid=1, all PROC.cwd = 0
	//MINODE minode[100]; all with refCount=0
	//MINODE *root = 0;
	int i;

	//allocate memory for running process
	running = malloc(sizeof(PROC));
	
	//set the initial processes values to default
	proc[0].pid = 1;
	proc[0].uid = 0;
	proc[0].cwd = 0;

	proc[1].pid = 2;
	proc[1].uid = 1;
	proc[1].cwd = 0;

	//set the running process to proc zero
	running = &proc[0];

	//MINODE minode[100] all with refCount=0, no procs are alive so no refs exists, if hard shutdown didnt cleanup this will fix that
	for(i = 0; i < 100; i++)
	{
		minode[i].refCount = 0;
	}

	//MINODE *root = 0;
	root = 0;
}

//mount root file system, establish / and CWDs
/*
	to mount root we must first
	1) check the device
	2)read the superblock into buf by passing its block number, buf, and device to getblock()
	3)
*/
mount_root(char device_name[64])
{
	//char buf[1024] buffer for storing block of 1024 bytes
	char buf[1024];
	//open device for RW
	dev = open(device_name, O_RDWR);

	//check if open() worked
	if(dev < 0)
	{
		//if device is not null then we open it, otherwise it failed and we exit
		printf("ERROR: failed cannot open %s\n", device_name);
		exit(0);
	}
	//then we read the super block by calling get_block with BLOCK #1 = supperblock
	//read super block to verify it's an EXT2 FS
	get_block(dev, SUPERBLOCK, buf);							//read get_block
	
	//creates the super block pointer so we can access its struct fields
	sp = (SUPER *)buf;
	//verify if it's an EXT2 FS
	if(sp->s_magic != 0xEF53)
	{
		printf("NOT AN EXT2 FS\n");
		exit(1);
	}

	//set some vars
	//get some info from the super block for use later
	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;

	//read group block for info
	//call get block on the next set of info by knowing that GDBLOCK number is 2 and store that in buf
	get_block(dev, GDBLOCK, buf);
	//creat the Group descriptor block pointer so we have access to its variables through a struct
	gp = (GD *)buf;

	//set the imap and bmap block numbers accordingly, so later we can use the imap and bmap to track our inodes and data blocks
	imap = gp->bg_inode_bitmap;
	bmap = gp->bg_block_bitmap;

	//set the inode table first block to inodesbeginblock because we have n group descriptors
	inodeBeginBlock = gp->bg_inode_table;

	//get root inode, root inode is block number two since it is the first group descriptor block as well
	root = iget(dev, 2);


	//let cwd of both p0 and p1 point at the root minode (refCount=3), root and cwd are both minode ptrs now.
	proc[0].cwd = root;
	proc[1].cwd = root;

	root->refCount = 3;

	//we are now mounted since our cwd in our running and idle procs points to the root inode of the FS, IE group descriptor[0]
	printf("%s mounted\n", device_name);
}
//print_info(temp_mip, temp);
//print information for file or dir
void print_info(MINODE *mip, char *name)
{
	int i;
	INODE *ip = &mip->INODE;

	char *permissions = "rwxrwxrwx";

	//information for each file
	u16 mode   = ip->i_mode;
    u16 links  = ip->i_links_count;
    u16 uid    = ip->i_uid;
    u16 gid    = ip->i_gid;
    u32 size   = ip->i_size;

	char *time = ctime( (time_t*)&ip->i_mtime);
	//remove \r from time
	time[strlen(time) - 1] = 0;

	switch(mode & 0xF000)
	{
		case 0x8000:  putchar('-');     break; // 0x8 = 1000
        case 0x4000:  putchar('d');     break; // 0x4 = 0100
        case 0xA000:  putchar('l');     break; // oxA = 1010
        default:      putchar('?');     break;
	}

	//print the permissions, neato
	for(i = 0; i < strlen(permissions); i++)
		putchar(mode & (1 << (strlen(permissions) - 1 - i)) ? permissions[i] : '-');

	//print the rest, links, gid, uid, size, time, name
	printf("%4hu %4hu %4hu %8u %26s  %s\n", links, gid, uid, size, time, name);
	strcat(teststr, name);
	strcat(teststr, " ");

	//done
	return;
}

//print directory
//Goes through the directory and runs print info for every dp
void print_dir(MINODE *mip)
{
	int i;
	DIR *dp;
	char *cp;
	char buf[1024], temp[1024];
	INODE *ip = &mip->INODE;
	MINODE *temp_mip;

	printf("printing directory\n");

	//cycles through every block
	for(i = 0; i < ip->i_size/1024; i++)
	{
		//if the block # is zero we have a problem
		if(ip->i_block[i] == 0)
			break;

		//get the block at the inodes first memblock slot
		get_block(dev, ip->i_block[i], buf);
		//cast it as a dir to access its info
		dp = (DIR*)buf;
		//set our ref counter
		cp = buf;

		//print the contents
		while(cp < buf + BLKSIZE)
		{
			//copy the name of the file into temp
			strncpy(temp, dp->name, dp->name_len);
			//add a null terminator at the end
			temp[dp->name_len] = 0;

			//need to print file permissions and such
			//printf("%s\n", temp);

			//temp mip holds the current inodes value for access
			temp_mip = iget(dev, dp->inode);
			//if a valid inode
			if(temp_mip)
			{
				//call print info on the current Inode, passing its name as well
				print_info(temp_mip, temp);
				//put the temp node back
				iput(temp_mip);
			}
			else
				printf("ERROR: Cannot print info for Minode\n");

			//move the pointers to advance to the next element
			memset(temp, 0, 1024);
			//increment cp by record length
			cp += dp->rec_len;
			//cast again as dir entry
			dp = (DIR*)cp;
		}
	}

	printf("\n");
}

//prints everything located at the pathname
//Calls printdir which calls printinfo for a given pathname
//The main function of ls is to get the pathname and check for certain cases
//Finds the ino of the pathname
void ls(char *pathname)
{
	int ino, offset;
	MINODE *mip = running->cwd;
	char name[64][64], temp[64];
	char buf[1024];

	//ls cwd
	if(!strcmp(pathname, ""))
	{
		//print_dir(mip->INODE);
		//pass the cwd mip to print_dir
		print_dir(mip);
		return;
	}

	//ls root dir
	if(!strcmp(pathname, "/"))
	{
		//print_dir(root->INODE);
		print_dir(root);
		return;
	}

	//if there's a pathname, ls pathname
	if(pathname)
	{
		//check if path starts at root
		if(pathname[0] == '/')
		{
			//set mip to the root pointer otherwise it will be set to relative path
			mip = root;
		}

		//search for path to print
		//returns the ino for the specified path
		ino = getino(mip, pathname);
		if(ino == 0)
		{
			//did not find the ino
			return;
		}

		//return the inode of the path element, set to mip
		mip = iget(dev, ino);
		//check if we are ls'ing a dir or a file type using macro functions
		if(!S_ISDIR(mip->INODE.i_mode))
		{
			printf("%s is not a directory!\n", pathname);
			strcpy(teststr, pathname);
			strcat(teststr, " is not a directory!");
			//return after putting back mip
			iput(mip);
			return;
		}

		//print_dir(mip->INODE);
		//call print dir on the mip specified previously
		print_dir(mip);
		iput(mip);
	}
	else
	{
		//print root dir
		//print_dir(root->INODE);
		//prints root dir if no case hits
		print_dir(root);
	}
}

//change cwd to pathname. If no pathname, set cwd to root.
//Checks some casses, ultimatly sets running->cwd to the inode of the pathname
void cd(char *pathname)
{
	int ino = 0;
	//set mip to current dir ptr
	MINODE *mip = running->cwd;
	//resulting directory
	MINODE *newmip = NULL;

	if (!strcmp(pathname, "")) //If pathname is empty CD to root
	{
		//root is now cwd minode
		running->cwd = root;
		return;
	}

	if (!strcmp(pathname, "/"))
	{
		//root is now cwd minode
		running->cwd = root;
		return;
	}

	printf("Path = %s\n", pathname);
	//returns the ino of path
	ino = getino(mip, pathname);
	if(ino == 0)
	{
		//failed to retrieve inode
		printf("The directory %s does not exist!\n", pathname);
		return;
	}

	//set newmip to the inode of the pathname
	newmip = iget(dev, ino);
	if(!S_ISDIR(newmip->INODE.i_mode))
	{
		//check to make sure this is a directory otherwise we failed
		printf("%s is not a directory!\n", pathname);
		iput(newmip);
		return;
	}

	//newmip is a directory so we set it and put back with iput
	running->cwd = newmip;
	iput(newmip);
	return;
}

//iput all DIRTY minodes before shutdown
int quit()
{
	int i;

	//go through all the minodes
	for(i = 0; i < NMINODE; i++)
	{
		//check if used and dirty
		if(minode[i].refCount > 0 && minode[i].dirty == 1)
		{
			minode[i].refCount = 1;
			iput(&minode[i]);
		}
	}

	printf("Goodbye!\n");
	exit(0);
}
