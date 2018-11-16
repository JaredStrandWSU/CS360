
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

#include <math.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

#define BLOCK_SIZE        1024
#define BLKSIZE           1024

#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FS
#define SUPERBLOCK        1
#define GDBLOCK           2
#define ROOT_INODE        2

#define BBITMAP           3 //bitmap of all the data blocks
#define IBITMAP           4 //bitmap of all the inodes
#define INODEBLOCK        5 //inode table, where all the free inodes live
#define INODE_START_POS   10 //block number where the inodes all can be referenced from

// Default dir and regulsr file modes for linux mode 
#define DIR_MODE          0040777
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define BUSY              1
#define READY             2
#define KILLED            3

// Table sizes
#define NMINODE           100
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT              50

// Open File Table
typedef struct Oft{
  int   mode;
  int   refCount;
  struct Minode *inodeptr;
  long  offset;
} OFT;

// PROC structure
typedef struct Proc{
  int   uid;
  int   pid;
  int   gid;
  int   ppid;
  int   status;

  struct Minode *cwd;
  OFT   *fd[NFD];

  struct Proc *next;
  struct Proc *parent;
  struct Proc *child;
  struct Proc *sibling;
} PROC;

// In-memory inodes structure, memory inodes contain an inode within them and the device, inumber, refcount and dirt and mounted
typedef struct Minode{
  INODE INODE;               // disk inode
  int   dev, ino;
  int   refCount;
  int   dirty;
  int   mounted;
  struct Mount *mountptr;
  char     name[128];           // name string of file, this is the name of the memory INODE
} MINODE;

// Mount Table structure, inside of every memory inode, this struct contains the information about the number of inodes, number of blocks, dev name and status(busy)
// this struct contains the name of the mount and mounted device name i think?
typedef struct Mount{
        int    ninodes;
        int    nblocks;
        int    dev, busy;
        struct Minode *mounted_inode;
        char   name[256];
        char   mount_name[64];
} MOUNT;

//global variables for all c files
//an array of memory INODES containing 100 main INODES on the disk
MINODE minode[NMINODE];
//root is a pointer to memory INODE, root will be used to reference the main memory Inode
MINODE *root;
//proc is an array of proc structs and running is a pointer to a proc structure
PROC   proc[NPROC], *running;
//an array of mount table structs called mount tab
MOUNT  mounttab[5];

char names[64][128],*name[64];
//third is a char array used in the input
char third [64];
//openfiletable is an array of 50 oft's 
OFT OpenFileTable[NOFT];
int fd, dev, n;
int nblocks, ninodes, bmap, imap, inode_start, iblock;
int inodeBeginBlock;
char pathname[256], parameter[256];
char teststr[1024] = "";
int DEBUG;
