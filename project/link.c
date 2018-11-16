//link oldfilename newfilename
//This makes newfile linked to oldfile
//This makes them have the same ino
//dirname is parent, basename is child
void my_link(char *path)
{
	char old[64], new[64], temp[64];
	char link_parent[64], link_child[64];
	int ino;
	int p_ino;
	MINODE *mip;
	MINODE *p_mip;
	INODE *ip;
	INODE *p_ip;

	//Checks old and new to make sure not empty
	if(!strcmp(path, ""))
	{
		printf("ERROR: no old file\n");
		return;
	}

	if(!strcmp(third, ""))
	{
		printf("ERROR: no new file\n");
		return;
	}

	//set new names to the params
	//call the old path old and the third param new link
	strcpy(old, path);
	strcpy(new, third);

	//get oldfilename's inode
//*
	ino = getino(running->cwd, old);
	//mip points at the inode of old
	mip = iget(dev, ino);

	//verify old file exists
	if(!mip)
	{
		printf("ERROR: %s does not exist\n", old);
		return;
	}
	//Verify it is a file
	if(S_ISDIR(mip->INODE.i_mode))
	{
		printf("ERROR: can't link directory\n");
		return;
	}
	printf("got it!\n");

	//get new's dirname, news parent
	if(!strcmp(new, "/"))
	{
		//set / to the link parent path
		strcpy(link_parent, "/");
	}
	else
	{
		//set link parent name to new's parent
		strcpy(temp, new);
		strcpy(link_parent, dirname(temp));
	}

	//get new's basename, this gives us the name of the file to create
	strcpy(temp, new);
	strcpy(link_child, basename(temp));

	printf("after stuff new is %s\n", new);

	//get new's parent
	printf("parent %s\n", link_parent);
//*
	p_ino = getino(running->cwd, link_parent);
	//mip now points at the new links parent inode
	p_mip = iget(dev, p_ino);

	//verify that link parent exists
	if(!p_mip)
	{
		printf("ERROR: no parent\n");
		return;
	}

	//verify link parent is a directory
	if(!S_ISDIR(p_mip->INODE.i_mode))
	{
		printf("ERROR: not a directory\n");
		return;
	}

	//verify that link child does not exist yet
	if(getino(running->cwd, new))
	{
		printf("ERROR: %s already exists\n", new);
		return;
	}

	//Enter the name for the newfile into the parent dir
	printf("entering name for %s\n", link_child);

	//This ino is the ino of the old file
	//This is how it is linked
	//p_mip is the parent of the new file, ino is the ino of old file, link_child is name of new link in a different dir
//*
	enter_name(p_mip, ino, link_child);

	ip = &mip->INODE;

	//increment the link count
	ip->i_links_count++;
	mip->dirty = 1;

	p_ip = &p_mip->INODE;

	p_ip->i_atime = time(0L);
	//Set dirty and iput
	p_mip->dirty = 1;

	iput(p_mip);

	iput(mip);

	printf("end of link\n");
	return;
}

//get ino of old link
//creat file in new path with old ino