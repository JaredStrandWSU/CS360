
//write function
int my_write(int fd, char buf[], int nbytes)
{
	int i, j;
	int *ip;
	int counter = 0;
	int remain;
	int lbk, startByte;
	int blk;
	int indirect_blk, indirect_off;

	//open file table pointer 
	OFT *oftp;
	MINODE *mip;
	//write buff is the block that holds the data chunk from buf to be written.
	char write_buf[1024];
	//cq and cp now point to the beginning of the string we want to write
	char *cp, *cq = buf;

	//Check to make sure the fd can exist
	if(fd < 0 || fd >= NFD)
	{
		printf("ERROR: invalid file descriptor\n");
		return;
	}

	//Goes through the OpenFileTable to find the fd
	for(i = 0; i < NOFT; i++)
	{
		//checks to see where the fd we are referenceing is in the oft so as to point to it as a oft struct 
		if(OpenFileTable[i].inodeptr == running->fd[fd]->inodeptr)
		{
			oftp = running->fd[fd];
			break;
		}
	}

	//Ensures the fd is open for write
	if(!oftp || (oftp->mode != 1 && oftp->mode != 2 && oftp->mode != 3)) //anything other than zero
	{
		printf("ERROR: wrong file mode for write\n");
		return;
	}

	//Gets the minode of the oftp
	//mip now points at the direct inode of the file
	mip = oftp->inodeptr;

	printf("About to write to file\n");

	//Writes while there are more bytes to be written
	while(nbytes > 0)
	{
		//lbk is now pointing at the beginning of the current block of the inode
		lbk = oftp->offset / BLKSIZE;
		//startbyte points at the offset of the current block
		startByte = oftp->offset % BLKSIZE;

		//check the direct blocks
		if(lbk < 12)
		{
			//if the block is null no data in there
			if(mip->INODE.i_block[lbk] == 0)
			{
				//allocate a block here for writing to
				mip->INODE.i_block[lbk] = balloc(mip->dev);
			}
			//set block to the memory block in the logical sequence of the inode block
			blk = mip->INODE.i_block[lbk];
		}
		//if lbk is 12th or less than 256 blocks ahead, we need to check for space to write indirectly
		else if(lbk >= 12 && lbk < 256 + 12)
		{
			//indirect

			//check if indirect block, else we need to allocate it
			//if the 13th block is null then we need to indirectly grab the next available block by allocating
			if(!mip->INODE.i_block[12])
			{
				//allocate a block in this space
				mip->INODE.i_block[12] = balloc(mip->dev);

				//fill the new block with 0's
				//store the nullblock in write_buf
				get_block(mip->dev, mip->INODE.i_block[12], write_buf);
				//for 1024 bytes, set them to zero
				for(i = 0; i < BLKSIZE; i++)
					write_buf[i] = 0;
				//put the now empty block back in position
				put_block(mip->dev, mip->INODE.i_block[12], write_buf);
			}
			//get the free block from memory location and store in write buf/ guaranteed to be all 0's
			get_block(mip->dev, mip->INODE.i_block[12], write_buf);

			//cast write buf as an int pointer and add the logical block number - 12 for indirect location
			//this sets ip to hold the address of the block we are going to write to.
			ip = (int*)write_buf + lbk - 12;
			//blk is now set to the indirect block number where we wish to write to
			blk = *ip;

			//if data block does not exist yet, have to allocate
			//CAUSING ISSUES??? bad logic maybe?
			if(blk == 0)
			{
				*ip = balloc(mip->dev);
				blk = *ip;
			}
		}
		else
		{
			//double indirect

			//same stuff, if it doesn't exist, we need to allocate it
			//should be mip->INODE.i_block[256]?
			if(mip->INODE.i_block[13] == 0)
			{
				mip->INODE.i_block[13] = balloc(mip->dev);
				//fill it with 0's!
				get_block(mip->dev, mip->INODE.i_block[13], write_buf);
				for(i = 0; i < BLKSIZE; i++)
					write_buf[i] = 0;
				put_block(mip->dev, mip->INODE.i_block[13], write_buf);
			}

			get_block(mip->dev, mip->INODE.i_block[13], write_buf);

			indirect_blk = (lbk - 256 - 12) / 256;
			indirect_off = (lbk - 256 - 12) % 256;

			//points at the block calculated
			ip = (int *)write_buf + indirect_blk;
			blk = *ip;

			//if no block yet, have to allocate it
			if(!blk)
			{
				ip = balloc(mip->dev);
				blk = *ip;

				get_block(mip->dev, blk, write_buf);
				for(i = 0; i < BLKSIZE; i++)
					write_buf[i] = 0;
				put_block(mip->dev, blk, write_buf);
			}

			get_block(mip->dev, blk, write_buf);

			//this sets ip to hold the address of the block we are going to write to.
			ip = (int*)write_buf + indirect_off;
			//blk now points at the blk location
			blk = *ip;

			//trying to allocate block and put back in mem if not a valid block number
			if(!blk)
			{
				*ip = balloc(mip->dev);
				blk = *ip;
				put_block(mip->dev, blk, write_buf);
			}
		}

		//get the block to write to and store in write buf
		get_block(mip->dev, blk, write_buf);
		//cp = write_buf plus the offset start byte, this is so we don't write over overrun data
		cp = write_buf + startByte;
		//remaining is calculated by the block we have minus the offset/used bytes
		remain = BLKSIZE - startByte;

		//Writes byte by byte trival
		while(remain > 0)
		{
			//cq points at the string, so copy each byte of the string to the mem block at cp
			*cp++ = *cq++;
			nbytes--;
			counter++;
			remain--;
			oftp->offset++;
			if(oftp->offset > mip->INODE.i_size)
				mip->INODE.i_size++;
			if(nbytes <= 0)
				break;
		}

		//put the full block back in mem, cycle back up if we still have remaining bytes to store
		put_block(mip->dev, blk, write_buf);
	}

	mip->dirty = 1;
	printf("Wrote %d char into file descripter fd=%d\n", counter, fd);
	return nbytes;
}

//write function, this is the one that gets called in main
void do_write(char *path)
{
	int i, fd, nbytes;
	//malloc a buf to hold the string as big as the input
	char *buf = (char*)malloc( (strlen(third) + 1) * sizeof(char*) );
	//open file table
	OFT *ofp;

	//Checks
	if(!path)
	{
		printf("ERROR: no path!\n");
		return;
	}

	if(!third)
	{
		printf("ERROR: no text to write\n");
		return;
	}

	//after making sure there's the right number of inputs,
	//take in the fd
	fd = atoi(path);

	for(i = 0; i < NOFT; i++)
	{
		//make sure it's in the global open file table
		ofp = &OpenFileTable[i];

		if(ofp->refCount == 0)
		{
			printf("ERROR: bad fd\n");
			return;
		}

		//have to verify the fd and its mode
		if(i == fd)
		{
			if(ofp->mode == 1 || ofp->mode == 2 || ofp->mode == 3)
				break;
			else
			{
				printf("ERROR: wrong mode for writing\n");
				return;
			}
		}
	}

	//copy the text to be written to a buffer
	//need to make read all text not just one word, add while loop
	strcpy(buf, third);
	nbytes = strlen(buf);
	printf("fd is %d\n", fd);
	my_write(fd, buf, nbytes);
	return;
}
