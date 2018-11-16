//Prints stats, simple as that.
void my_stat(char *path)
{
	int ino;
	MINODE *mip;
	INODE *ip;
	char *cp;
	char buf[1024];
	char name[64];
	char *my_atime;
	char *my_mtime;
	char *my_ctime;
	DIR *dp;

	//get the ino of the item in our cwd
	ino = getino(running->cwd, path);
	//mip set to the inode we selected
	mip = iget(dev, ino);

	strcpy(name, basename(path));

	//sets the ip to the actual inode for the chosen dir
	ip = &mip->INODE;

	printf("  File: %s\n", name);
	printf("  Size: %d\tBlocks: %12d ", ip->i_size, ip->i_blocks);
	if(S_ISDIR(ip->i_mode))
		printf("  Directory\n");
	else
		printf("  File\n");
	printf("Inode: %d Links:%d \n", ino, ip->i_links_count);

	my_atime = ctime( (time_t*)&ip->i_atime);
	my_mtime = ctime( (time_t*)&ip->i_mtime);
	my_ctime = ctime( (time_t*)&ip->i_ctime);

	printf("Access: %26s", my_atime);
	printf("Modify: %26s", my_mtime);
	printf("Change: %26s", my_ctime);

	//mark as dirty and then put
	mip->dirty = 1;
	iput(mip);

	return;
}
