//make dir parses the path,
//gets the ino of the current/parent directory
//calls my_mkdir which allocates a new inode, and adds the name to the parents data block
void make_dir(char path[124])
{
	int i, ino;
	MINODE *pmip;
	INODE *pip;

	char buf[1024];
	char temp1[1024], temp2[1024];
	char parent_name[1024], child_name[1024];

	//copy path so we don't destroy it
	strcpy(temp1, path);
	strcpy(temp2, path);

	//get parent and child name
	strcpy(parent_name, dirname(temp1));
	strcpy(child_name, basename(temp2));
	//printf("parent: %s\nchild: %s\n", parent_name, child_name);

	//get parent's ino
	ino = getino(running->cwd, parent_name);
	printf("ino is %d\n", ino);
	pmip = iget(dev, ino);
	pip = &pmip->INODE;

	//check if parent exists
	if(!pmip)
	{
		printf("ERROR: parent does not exist\n");
		return;
	}

	if(pmip == root)
		printf("in root\n");

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

	printf("running my_mkdir\n");
	//Call my_mkdir to make the dir, pass iun the pointer to parent inode and child_name which is basename
	my_mkdir(pmip, child_name);

	//increment the parents link count and adjust the time
	pip->i_links_count++;
	pip->i_atime = time(0L);
	pmip->dirty = 1;
	//set dirty to true and iput

	iput(pmip);

	return;
}

//enter_name(pmip, ino, child_name);
//puts the name into the parent directory, adds the link to the directory we just made into its parent folder so we can see it on ls
int enter_name(MINODE *mip, int myino, char *myname)
{
	int i;
	INODE *parent_ip = &mip->INODE;

	char buf[1024];
	char *cp;
	DIR *dp;

	//used for calcualting the desired lengths to optimize the memory blocks for holding file names
	int need_len = 0, ideal = 0, remain = 0;
	int bno = 0, block_size = 1024;

	//while the parent inode still has blocks potentially open, checks to see if the parent dir is full and needs to allocate more space
	for(i = 0; i < parent_ip->i_size / BLKSIZE; i++)
	{
		//if the parent block number is zero or null them somethings wrongs and we hav eto allocate new node
		if(parent_ip->i_block[i] == 0)
			break;

		//get the current parent node block number
		bno = parent_ip->i_block[i];

		//get the parents data block and put it into the buf for processing
		get_block(dev, bno, buf);

		//cast the parent data block into a directory pointer
		dp = (DIR*)buf;
		//set cp also to point a the same memory block as dp
		cp = buf;

		//need length, calculate the needed length from KC's algorithm based on the name of the new dir
		need_len = 4 * ( (8 + strlen(myname) + 3) / 4);
		printf("need len is %d\n", need_len);

		//step into last dir entry, why, because we are trying to findthe end of the data section for this block of the parent inode, 
		//this will continuously chunk out the directories in the parents block
		//This way we can find the last directory in the folder of the parent so as to compress the memory. 
		while(cp + dp->rec_len < buf + BLKSIZE)
		{
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}

		//last entry is thet last file underneath the parent inode.
		printf("last entry is %s\n", dp->name);
		cp = (char*)dp;

		//ideal length uses name len of last dir entry to calculate how to squeeze new entry into remaining space
		ideal = 4 * ( (8 + dp->name_len + 3) / 4);

		//reamining is the current records length like 1024-300 = 690 something remaining
		remain = dp->rec_len - ideal;
		printf("remain is %d\n", remain);
		
		//if the remaining amount is greather than the needed length to store the new entry info then we will execute
		if(remain >= need_len)
		{
			//set rec_len to ideal, set the last entries length to ideal length
			dp->rec_len = ideal;

			//set the current pointer equal to the end of ideal, now we are setting dp to the end of the last record
			cp += dp->rec_len;
			dp = (DIR*)cp;

			//sets the dirpointer inode to the given myino
			dp->inode = myino;
			//we are now setting the length of the new directory
			dp->rec_len = block_size - ((u32)cp - (u32)buf);
			printf("rec len is %d\n", dp->rec_len);
			//set the variable char length of the name field to the length of the path name
			dp->name_len = strlen(myname);

			dp->file_type = EXT2_FT_DIR;
			//sets the dp name to the given name
			strcpy(dp->name, myname);
			//new dir is now ready to be placed into memory and completely done
			//puts the block
			put_block(dev, bno, buf);

			return 1;
		}
	}

	printf("number is %d\n", i);

	//no space in existing data blocks, time to allocate in next block
	bno = balloc(dev);
	//set parents next i_block to the newly allocated block number
	parent_ip->i_block[i] = bno;

	//reset the size since we just allocated an extra block
	parent_ip->i_size += BLKSIZE;
	mip->dirty = 1;

	//get the new block into buff to work with it
	get_block(dev, bno, buf);

	//cast buf as a dir entry
	dp = (DIR*)buf;
	cp = buf;

	printf("dir name is %s\n", dp->name);
	//set the inode number of the new block to the child ino
	dp->inode = myino;
	//set the default record length
	dp->rec_len = 1024;
	//set the variable name length to the length of the file
	dp->name_len = strlen(myname);
	//set file type
	dp->file_type = EXT2_FT_DIR;
	//copy the name of the dir
	strcpy(dp->name, myname);
	//put that block back and call it a day
	put_block(dev, bno, buf);

	return 1;
}

//accepts the parent directory pointer and the childs name
//allocates a new inode and block for the new dir
//gets the Minode and inode for the newly allocated ino
//set all the correct fields for the new dir, including . and .. links
//it has two free slots, first block holds block number and rest are 0 -> 12 are zero
void my_mkdir(MINODE *pmip, char *child_name)
{
	//allocate a new blocka and inode from the device
	int ino = ialloc(dev);
	int bno = balloc(dev);
	int i;

	printf("device is %d\n", dev);
	printf("ino is %d\nbno is %d\n", ino, bno);

	//get the inode and set mip to point at it
	MINODE *mip = iget(dev, ino);
	//set the inode pointer at the actualy inode
	INODE *ip = &mip->INODE;

	char *cp, buf[1024];
	DIR *dp;

	ip->i_mode = 0x41ED; // OR 040755: DIR type and permissions
	//printf("mode is %d\n", ip->i_mode);
	ip->i_uid  = running->uid; // Owner uid
	//printf("uid is %d\n", ip->i_uid);
	ip->i_gid  = running->gid; // Group Id
	//printf("gid is %d\n", ip->i_gid);
	//We set the size to blksize to because that is the size of a dir
	ip->i_size = BLKSIZE;// Size in bytes
	ip->i_links_count = 2;// Links count=2 because of . and ..
	ip->i_atime = time(0L);// set access time to current time
	ip->i_ctime = time(0L);// set creation time to current time
	ip->i_mtime = time(0L);// set modify time to current time

	//This is for . and ..
	ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks
	ip->i_block[0] = bno;             // new DIR has one data block

	//set all blocks to 0
	for(i = 1; i < 15; i++)
	{
		ip->i_block[i] = 0;
	}

	mip->dirty = 1;//set dirty to true and iput
	iput(mip);

	//create data block for new DIR containing . and ..

	//gets the first block of the new inode and puts in buf, setting this new inodes dir variables inlcuding self . and .., 
	get_block(dev, bno, buf);

	dp = (DIR*)buf;
	cp = buf;

	//setting the new inodes number
	dp->inode = ino;
	//setting the new inodes length to the default length
	dp->rec_len = 4 * (( 8 + 1 + 3) / 4);
	//setting the name length to 1 char
	dp->name_len = strlen(".");
	//setting the file type field
	dp->file_type = (u8)EXT2_FT_DIR;
	//setting the main name of this dir to . so it can hold child dirs and maintain dir structure
	dp->name[0] = '.';
	//first block of the new child node is set to . for name
	cp += dp->rec_len;
	dp = (DIR*)cp;

	//This portion is for the parent (..)
	//this is the second block of the new inode, second compressed block
	//setting the second block equal to the parents ino so as to locate it
	dp->inode = pmip->ino;
	//setting the record length since we took space to add . and .. blocks
	dp->rec_len = 1012;//will always be 12 in this case
	printf("rec_len is %d\n", dp->rec_len);
	//name length is two bytes for calculating space needed
	dp->name_len = strlen("..");
	//set file type to dir
	dp->file_type = (u8)EXT2_FT_DIR;
	//set first and second name bytes to the ..
	dp->name[0] = '.';
	dp->name[1] = '.';

	//write buf to disk block bno
	//writes the now directory block back to disk
	put_block(dev, bno, buf);

	//enter name entry into parent's directory
	enter_name(pmip, ino, child_name);

	return 1;
}
