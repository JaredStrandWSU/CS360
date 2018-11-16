
// print the pathname of current working directory
int my_pwd()
{
    // Allocate memory for the path of the cwd
    char path[128] = "";
	char temp[128] = "";
	char name[64] = "";
	int ino, parent_ino;
	MINODE *mip = running->cwd;

	//check if at root
	if(mip == root)
	{
		printf("/\n");
        strcpy(teststr, "/");
		return;
	}

	//keep doing until reaches root
    while(mip != root)
	{
		//find ino of cwd and parent, looks at the . and .. blocks to get ino's
		findino(mip, &ino, &parent_ino);
		//set mip to parent
		mip = iget(dev, parent_ino);
		//find name using parent and ino
		findmyname(mip, ino, name);

		//copy so we can have it in the right order
		strcpy(temp, path);
		strcpy(path, "");
		//add to path
		strcat(path, "/");
		strcat(path, name);
		strcat(path, temp);
	}
	//print the cwd
	printf("%s\n", path);
    strcpy(teststr, path);

    return 1;
}
