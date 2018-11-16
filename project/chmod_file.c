//chmod changes the file mode of a given file name
void chmod_file(char path[124])
{
  int ino = 0;
  int newmode = 0;
  MINODE *mip = running->cwd;
  INODE *ip = NULL;

  //We have to have a given mode
  if (!strcmp(third, ""))
  {
    printf("No mode given!\n");
    return;
  }
  //This converts the string from third to an octal int for the permissions
  newmode = (int) strtol(third, (char **)NULL, 8);

  printf("path = %s\n", path);
  //Get the ino of the given path
  //do an iget on the ino
  ino = getino(mip, path);
  if (ino)                          
    mip = iget(dev, ino);
  else
  {
    strcpy(third, "");
    return;
  }
  ip = &mip->INODE;
                                                                
  //ip->i_mode & 0xF000 clears the current mode and ORs it with the new mode
  //Leaving us with the new permissions
  ip->i_mode = (ip->i_mode & 0xF000) | newmode;

  //Set it to dirty and iput
  mip->dirty = 1;
  iput(mip);
  strcpy(third, "");
}
