/*
main function, user enters program and hits first 
//cp and write have issues with big files, due to indirection
*/
#include <stdio.h>
#include <stdlib.h>
#include "type.h"
#include "util.c"
#include "alloc_dealloc.c"
#include "pwd.c"
#include "mkdir.c"
#include "rmdir.c"
#include "creat_file.c"
#include "chmod_file.c"
#include "touch_file.c"
#include "mount_root.c"
#include "link.c"
#include "open.c"
#include "close.c"
#include "pfd.c"
#include "symlink.c"
#include "unlink.c"
#include "stat.c"
#include "cp.c"
#include "mv.c"
#include "lseek.c"
#include "read.c"
#include "write.c"
#include "cat.c"
#include "tests.c"

/*
LEVEL 1:
mount_root;
mkdir, rmdir, ls, cd, pwd;
creat, link, unlink, symlink
stat, chmod, touch;

LEVEL 2:
open, close, read, write
lseek, cat, cp, mv

LEVEL 3:
mount, unmount
File permission checking
*/

void menu()
{
	printf("COMMANDS:\n");
	printf("  mkdir\t\trmdir\t\tls\t\tcd\n");
	printf("  cd\t\tpwd\t\tcreat\t\tlink\n");
	printf("  unlink\tsymlink\t\tstat\t\tchmod\n");
	printf("  touch\t\tquit\t\tmenu\n");
}

int main()
{
	//this is an array of pointers to strings
	char *commands[23] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "link", "unlink", "symlink", "stat", "chmod", "touch", "quit", "menu", "open", "close", "pfd", "write", "cat", "read", "cp", "mv", "lseek"};
	//these are the variables to hold the mydisk name and the input string up to 128 chars, set to null initially
	char device_name[64], input[128] = "";
	//variable to determine if we are in testing mode
	int testing = 1;
	//char arrays to hold the command string and the pathname string, set to null iniitally
	char command[64], pathname[64] = "";

	//functions array to allow user to call 
	int (*functions[23])(char path[]);
	int i;

	functions[0] = make_dir;
	functions[1] = remove_dir;
	functions[2] = ls;
	functions[3] = cd;
	functions[4] = my_pwd;
	functions[5] = creat_file;
	functions[6] = my_link;
	functions[7] = my_unlink;
	functions[8] = my_symlink;
	functions[9] = my_stat;
	functions[10] = chmod_file;
	functions[11] = touch_file;
	functions[12] = quit;
	functions[13] = menu;
	functions[14] = open_file;
	functions[15] = my_close;
	functions[16] = my_pfd;
	functions[17] = do_write;
	functions[18] = my_cat;
	functions[19] = read_file;
	functions[20] = cp_file;
	functions[21] = mv_file;
	functions[22] = my_lseek;

	//prompt the user initally to input a storage device for the project
	printf("input device name: ");
	//device name is now set to the input string.
	scanf("%s", device_name);

	//mount device
	init();
	mount_root(device_name);
	if(testing == 1)
	{
		runTests();
	}
	menu();
	getchar();
	while(1)
	{
		printf("Command>");
		fgets(input, 128, stdin);

		//remove \r at end of input
		input[strlen(input) - 1] = 0;
		if(input[0] == 0)
			continue;

		//split up the input into the variables
		sscanf(input, "%s %s %s", command, pathname, third);

		for(i = 0; i < 23; i++)
		{
			if(!strcmp(commands[i], command))
			{
				(*functions[i])(pathname);
				break;
			}
		}

		if(i == 23)
			printf("invalid command\n");

		//reset variables
		strcpy(pathname, "");
		strcpy(command, "");
		strcpy(input, "");
		strcpy(third, "");
	}
	return 0;
}
