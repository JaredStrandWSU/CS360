
//Touch will call creat to create a file if touch is called on an non-existant file
//If the file exists, it will simply update its inodes time variable
void touch_file(char path[124])
{
  int ino;
  int newmode = 0;
  MINODE *mip = running->cwd;
  MINODE *touchmip = NULL;
  INODE *ip = NULL;
  char fullpath[128];
  strcpy(fullpath, path);

  //Checks
  if (!strcmp(path, ""))
  {
    printf("No pathname given!\n");
    return;
  }

  printf("path = %s\n", path);
  //searches for ino of pathname in current working dir
  ino = getino(running->cwd, path);

  if (ino != 0) //The target exists, touch it
  {
    printf("The target exists, touching...\n");
	//iget the inode to touch
    touchmip = iget(dev, ino);
    ip = &touchmip->INODE;
	//update the time
    ip->i_mtime = time(0L);
    touchmip->dirty = 1;
	//return home
    iput(touchmip);
	return;
  }
  else //The traget doesnt exist and we must make a file
  {
	  //creates a new file called name specified
    printf("The target does not exists, creating file...\n");
    creat_file(fullpath);
  }

  return;
}
