/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

static const char *hello_str = "Bello World!\n";
static const char *hello_path = "/hello";
static const char *fs_path = "/home/rgmolina/FS_FILE";

/*
Format of blocks

1st block of file
0-1 first block number (set to 0 if block is off)
2-3 next block number (set to 0 if block is last block)
4-103 file name
104-111 creation time
112-119 modify time
120-121 size
122 open bit
123-4094 actual data

2nd+ block of file
0-1 first block number
2-3 next block number
4-122 wasted space
123-4094 actual data

*/
// hello_getattr(const char *path, struct stat *stbuf)
	// Gets attributes, such as:
	// If what the user is requesting exists
	// If the user is requesting a file or a directory
	// The permissions of requested file/directory
	// Sets metadata in superblock

static int hello_getattr(const char *path, struct stat *stbuf)
{
	printf("top of getattr\n");
	int res = 0;
	FILE *fs;
	char filename[101];
	int locnum;
	int counter;
	char bitmap[65];
	char curblock[4096];
	time_t * timev;
	fs = fopen(fs_path, "r");
	fseek(fs, 5, SEEK_SET);
	fgets(bitmap, 65, fs);
	for(int i = 1; i<=500; i++){
		int j = i/8;
		char k = bitmap[j];
		int l = i - j * 8;
		k = (k & (1 << l)) >> l;
		if (k){
			printf("getattr k=1\n");
			fseek(fs, 4096 * i, SEEK_SET);
			fgets(curblock, 4096, fs);
			locnum = curblock[0] * 256 + curblock[1];
			if (locnum == i){
				strncpy(filename, curblock+4, 100);
				if (strcmp(path, filename) == 0){
					stbuf->st_mode = S_IFREG | 0666;
					stbuf->st_nlink = 1;
					timev = (time_t*)(&stbuf[104]);
					stbuf->st_atime = *timev;
					timev = (time_t*)(&stbuf[112]);
					stbuf->st_mtime = *timev;
					counter = curblock[120] * 256 + curblock[121];
					stbuf->st_size = counter;
					stbuf->st_blocks = counter / 4096 + 1;
					stbuf->st_blksize = 4096;
					break;
				}
			}
		}

	}
	fclose(fs);
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, hello_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	} else
		res = -ENOENT;
	return res;
}

// hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
// 	Checks if path is just “/”
	// 	If it is, then do the following:
	// 		Check if offset is 0
	// 		If yes
	// 			output “.”, “..”
	// 			Then loops through blocks, outputting each filename until buf is full.
	// 		If not
	// 			skip entries until reach offset, including “.” and “..”
	// 			Keep looping through blocks, outputting each filename until buf is full
	// 	If not
	// 		Check if file exists
	// 			If yes, output filename
	// 			If not, output empty buffer


static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);

	return 0;
}

// hello_open(const char *path, struct fuse_file_info *fi	)
// 	Loops through every block
// 	If filename is found, the file marked as opened for read/write/other access functions
// 	If filename is not found nothing happens

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

// hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
// 	Loops through every block
// 	If filename is found
// 	Check if marked as open by hello_open
// 	If not open, return 0
// 	If open
// 	Skip bytes until reach the offset
// 	Put bytes into buf until buf is full or until there are no more bytes in the file
// 	Return number of bytes put in buf
// 	If not found, return 0

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	printf("top of read\n");
	FILE *fs;
	char filename[101];
	int locnum;
	int counter;
	char bitmap[65];
	time_t * time;
	char curblock[4096];
	fs = fopen(fs_path, "r");
	fseek(fs, 5, SEEK_SET);
	fgets(bitmap, 65, fs);
	for(int i = 1; i<=500; i++){
		int j = i/8;
		char k = bitmap[j];
		int l = i - j * 8;
		k = (k & (1 << l)) >> l;
		if (k){
			printf("read k = 1\n");
			fseek(fs, 4096 * i, SEEK_SET);
			fgets(curblock, 4096, fs);
			locnum = curblock[0] * 256 + curblock[1];
			if (locnum == i){
				strncpy(filename, curblock+4, 100);
				if (strcmp(path, filename) == 0){
					//read a file
				}
			}
		}
	}
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

	return size;
}

// static int hello_write(const char *path, const char *buf, size_t size, off_t offset,
// 		      struct fuse_file_info *fi)
// {
// 	FILE *fs;
// 	char filename[101];
// 	int locnum;
// 	int counter;
// 	char bitmap[65];
// 	char time[4];
// 	char curblock[4096];
// 	fs = fopen(fs_path, "r");
// 	fseek(fs, 5, SEEK_SET);
// 	fgets(bitmap, 65, fs);
// 	for(int i = 1; i<=500; i++){
// 		int j = i/8;
// 		char k = bitmap[j];
// 		int l = i - j * 8;
// 		k = (k & (1 << l)) >> l;
// 		if (k){
// 			printf("unlink k = 1\n");
// 			fseek(fs, 4096 * i, SEEK_SET);
// 			fgets(curblock, 4096, fs);
// 			locnum = curblock[0] * 256 + curblock[1];
// 			if (locnum == i){
// 				strncpy(filename, curblock+4, 100);
// 				if (strcmp(path, filename) == 0){
// 	write to the file found
// 				}
// 			}
// 		}
// 	}
//
// 	return size;
// }

// hello_unlink(const char *path)
// 	Loops through every block
// 	If filename is found
// 	Go through the blocks the file takes up and recursively empty out blocks from the last block back
// 	Disable the corresponding bits in the bitmap
// 	If filename is not found nothing is done

static int hello_unlink(const char *path){
	printf("top of unlink\n");
	FILE *fs;
	char filename[101];
	int locnum;
	int counter;
	char bitmap[65];
	char time[4];
	char curblock[4096];
	fs = fopen(fs_path, "r");
	fseek(fs, 5, SEEK_SET);
	fgets(bitmap, 65, fs);
	for(int i = 1; i<=500; i++){
		int j = i/8;
		char k = bitmap[j];
		int l = i - j * 8;
		k = (k & (1 << l)) >> l;
		if (k){
			printf("unlink k = 1\n");
			fseek(fs, 4096 * i, SEEK_SET);
			fgets(curblock, 4096, fs);
			locnum = curblock[0] * 256 + curblock[1];
			if (locnum == i){
				strncpy(filename, curblock+4, 100);
				if (strcmp(path, filename) == 0){
					//delete file and all the blocks it takes up
					//
					//
					return 0;
				}
			}
		}
	}

	return 0;
}

// hello_create(const char *path, mode_t mode, struct fuse_file_info *fi)
// 	Loops through every block
// 	If filename already exists, mark the file as opened for read/write/other access functions
// 	If filename does not exists next loop starts
// 	Loops through bitmap
// 	If free block is found
// 	Create and initialize the metadata for the new block
// 	Turn the corresponding bit in the bitmap on
// 	Mark the file as opened for read/write/other access functions
// 	If no free blocks are available error message is displayed

static int hello_create(const char *path, mode_t mode, struct fuse_file_info *fi){
	printf("top of create\n");
	FILE *fs;
	char filename[101];
	int locnum;
	int counter;
	char bitmap[65];
	time_t * timev;
	char curblock[4096];
	time_t rawtime;
	fs = fopen(fs_path, "r+");
	fseek(fs, 5, SEEK_SET);
	fgets(bitmap, 65, fs);
	for(int i = 1; i<=500; i++){
		int j = i/8;
		char k = bitmap[j];
		int l = i - j * 8;
		k = (k & (1 << l)) >> l;
		if (k){
			printf("create k=1\n");
			fseek(fs, 4096 * i, SEEK_SET);
			fgets(curblock, 4096, fs);
			locnum = curblock[0] * 256 + curblock[1];
			if (locnum == i){
				strncpy(filename, curblock+4, 100);
				if (strcmp(path, filename) == 0){
					if (curblock[122] == 1){
						return 1;
					}
					curblock[122] = 1;
					fseek(fs, 4096 * i, SEEK_SET);
					fputs(curblock, fs);
					return 0;
				}
			}
		}
	}
	for(int i = 1; i<=500; i++){
		int j = i/8;
		char k = bitmap[j];
		int l = i - j * 8;
		k = (k & (1 << l)) >> l;
		if (!k){
			// printf("create k not 1\n");
			// memset(curblock, 0, sizeof(curblock)-1);
			// printf("1");
			// curblock[0] = i / 256;
			// printf("1");
			// curblock[1] = i % 256;
			// printf("1");
			// timev = (time_t*)curblock[104];
			// printf("1");
			// time(*timev);
			// printf("1");
			// timev = (time_t*)curblock[112];
			// printf("1");
			// time(*timev);
		}
	}
	fclose(fs);
	return 0;
}

static struct fuse_operations hello_oper = {
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open			= hello_open,
	.read			= hello_read,
//	.write 		= hello_write,
//	.setattr 	=	hello_setattr,
	.unlink 	= hello_unlink,
	.create 	= hello_create,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &hello_oper, NULL);
}
