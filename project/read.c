//Read: path is fd, third is nbytes
int read_file(char *path)
{
	//Convers the number of bytes we want to read from a string to an int
	int nbytes = atoi(third), actual= 0;
	int fd = 0;
	//Set the size of buf to number of bytes we wanna read plus 1
	//Plus one for the null terminator
	char buf[nbytes + 1];

	MINODE *mip;
	INODE* ip;

	//buff now contains empty string
	strcpy(buf, "");

	printf("Reading file!\n");
	//Checks the fd
	if (!strcmp(path, ""))
	{
		printf("No fd specified!\n");
		return;
	}
	//Converts the fd from a string to an int, for direct access
	fd = atoi(path);
	//Ensures there is a number of bytes to be read specified
	if (!strcmp(third, ""))
	{
		printf("No byte amount specified!\n");
		return;
	}

	//my_read will return the number of bytes that were actually read
	//This is important because the user may ask to print more bytes than there are
	actual = my_read(fd, buf, nbytes);

	if (actual == -1)
	{
		printf("ERROR: Couldn't read file!\n");
		strcpy(third, "");
		return;
	}

	buf[actual] = '\0';
	printf("%s\n", buf);
	return actual;
}

//Does the actual Reading
//outputs to buf as an ouput parameter and returns the number of bytes that were read
//large files no working
int my_read(int fd, char *buf, int nbytes)
{
	MINODE *mip;
	OFT *oftp;

	int count = 0;
	int lbk, blk, startByte, remain, ino;
	int avil;
	int *ip;

	int indirect_blk;
	int indirect_off;

	//int buffer for indirection
	int buf2[BLKSIZE];

	//pointers for data reading and bufs for storage
	char *cq, *cp;
	char readbuf[1024];
	char temp[1024];

	//sets the oftp to the correct fd, using the user specified fd
	oftp = running->fd[fd];
	//mip now points directly at the inode of the oft for reading
	mip = oftp->inodeptr;

	//Calculates the available amount of bytes to be read
	//This is from the size of the file minus the given offset
	avil = mip->INODE.i_size - oftp->offset;
	//cq now points at the same place as buf, the return string we are reading
	cq = buf;

	//Loops While there are more bytes available and we want to read more bytes
	while(nbytes && avil)
	{
		//Mailmans to logical bloack number and startbyte in that block from offset
		//logical block is calculated from the oft's offset using integer division
		lbk = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;

		//If logical block is less than 12 we are only in the direct blocks
		//get the block of data and put in blk is the goal, then we will read the block and store in buf
		if(lbk < 12)
		{
			//map logical block lbk to physical blk
			blk = mip->INODE.i_block[lbk];
		}
		else if(lbk >= 12 && lbk < 256 + 12)
		{
			printf("DOING INDIRECT\n");
			//indirect blocks
			//They are located at iblock 12
			get_block(mip->dev, mip->INODE.i_block[12], readbuf);

			ip = (int *)readbuf + lbk - 12;
			blk = *ip;
		}
		else
		{
			printf("DOING DOUBLE INDIRECT\n");
			//double indirect blocks
			//They are located at iblock 13
			get_block(mip->dev, mip->INODE.i_block[13], readbuf);

			indirect_blk = (lbk - 256 - 12) / 256;
			indirect_off = (lbk - 256 - 12) % 256;

			ip = (int *)readbuf + indirect_blk;

			get_block(mip->dev, *ip, readbuf);

			ip = (int *)readbuf + indirect_off;
			blk = *ip;
		}

		//get the data block into readbuf[BLKSIZE]
		get_block(mip->dev, blk, readbuf);

		//set cp to readbuf + start byte since startbyte is the offset of the block we're reading
		cp = readbuf + startByte;

		//calculate the remaining bytes to read.
		remain = BLKSIZE - startByte;

		//old way of doing this (one byte at a time)
		while(remain > 0)
		{
			//copy byte fromn readbuf into buf
			*cq++ = *cp++;
			oftp->offset++;
			count++;
			avil--;
			nbytes--;
			remain--;
			//break if off by one error
			if(nbytes <= 0 || avil <= 0)
				break;
		}

		/* My attempt at dynamic read of bytes, said it was required

		if(nbytes < avil)
		{
			strncpy(temp, cp, nbytes);
			strcat(cq, temp);
			oftp->offset += nbytes;
			count += nbytes;
			avil -= nbytes;
			nbytes -= nbytes;
			remain -= nbytes;
		}
		else
		{
			strncpy(temp, cp, avil);
			strcat(cq, temp);
			oftp->offset += avil;
			count += avil;
			avil -= avil;
			nbytes -= avil;
			remain -= avil;
		}

		*/

		//if one data block is not enough, loop back to OUTER while for more...
	}
	printf("\nmyread: read %d char from file  descriptor %d\n", count, fd);
	printf("remain is: %d\n", remain);
	printf("nbytes is: %d\n", nbytes);
	return count;
}
