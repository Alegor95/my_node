#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include "mynode.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
//Converters
static int node_to_stat(struct my_node *buffer, struct stat *stbuf){
  stbuf->st_ino=buffer->number;
  stbuf->st_dev = buffer->device_id;
  stbuf->st_mode = buffer->mode;
  stbuf->st_nlink = 1;
  stbuf->st_uid = buffer->owner_id;
  stbuf->st_gid = buffer->owner_group_id;
  stbuf->st_atime = buffer->a_time;
  stbuf->st_mtime = buffer->m_time;
  stbuf->st_ctime = buffer->c_time;
}
//Fuse methods
//Remove file
int node_unlink(const char *path)
{
	printf("node_unlink: удаление файла %s\n", path);
	int number = getNumberByPath(path);
	if (number < 0){
		printf("node_unlink: файл не существует %s\n", path);
		return -ENOENT;
	}
	removeNode(number);
}
//Create file
static int node_mknod(const char *path, mode_t mode, dev_t dev)
{
	int res = 0;
	printf("node_mknod: создание файла %s\n", path);
	struct my_node new_node = (struct my_node){0};
	fillNode(&new_node, 0, (unsigned int)mode, (unsigned int)dev, "");
	if(addNode(&new_node)){
		printf("node_nknod: ошибка при создании файла%s\n", path);
		return -EIO;
	}
	return res;
}
//Get file information
static int node_getattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	//Get inode number
	printf("node_getattr: получение информации о файле %s\n", path);
  struct my_node buffer = (struct my_node){0};
	if (getNodeByPath(path, &buffer)){
		printf("node_getattr: не удалось получить информацию о файле %s\n", path);
		return -ENOENT;
	}
	//Parse node to stat
	node_to_stat(&buffer, stbuf);
  //Yuppi!
	return 0;
}

static int node_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	//if (strcmp(path, "/") != 0)
	//	return -ENOENT;
	printf("node_readdir: получение файлов в директории %s\n", path);
  //
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	int i;
	for (i=1; i < 10; i++){
		struct my_node buffer = (struct my_node){0};
		if (!getNodeByNumber(i, &buffer)){
		  char filename[20];
		  sprintf(filename, "%d", i);
			printf("node_readdir: добавление файла %s в список директории\n", filename);
		  filler(buf, filename, NULL, 0);
		}
	}
	return 0;
}

static int node_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int node_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
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

static struct fuse_operations hello_oper = {
	.getattr	= node_getattr,
	.readdir	= node_readdir,
	.open		= node_open,
	.read		= node_read,
	.mknod = node_mknod,
	.unlink = node_unlink
};

int main(int argc, char *argv[])
{
	mynode_initialization();
	return fuse_main(argc, argv, &hello_oper, NULL);
}
