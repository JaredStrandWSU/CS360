
//cat a file given the path
void my_cat(char *path)
{
	int n, i;
	int fd = 0;
	char buf[1024];
	int size = 1024;

	int testing = 0;
	
	//check for path
	if(!path)
	{
		printf("ERROR: missing path\n");
		return;
	}

	//make sure open mode is read, change it if not
	strcpy(third, "0");

	//open with 0 for RD
	//opens the fd for the given path for mode 0
	fd = open_file(path);
	//print the file descriptor information for user to see
	my_pfd("");

	//printf("fd is %d\n", fd);
	//n is actualy number of bytes read from my_read, and buf contains the data
	while((n = my_read(fd, buf, size)))
	{
		//printf("count is %d\n", n);
		
		//null terminate the buffer
		buf[n] = '\0';

		i = 0;
		//print each char in the buffer, this is to handle \n
		while(buf[i])
		{
			putchar(buf[i]);
			if(buf[i] == '\n')
				putchar('\r');
			i++;
		}
	}
	
	printf("\n\r");
	close_file(fd);

	return;
}
