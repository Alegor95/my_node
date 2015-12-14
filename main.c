#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mynode.h"
#include "node_content.h"

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
	stbuf->st_size = buffer->content_size;
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
		printf("node_mknod: ошибка при создании файла%s\n", path);
		return -EIO;
	}
	//Parse it
	char *directory_path; char *filename;
	filename = strrchr(path, '/')+1;
	int dpath_len = (filename-path);
	directory_path = (char *)malloc(dpath_len+1);
	strncpy(directory_path, path, dpath_len);
	directory_path[dpath_len] = 0;
	//Get directory node
	struct my_node dir = (struct my_node){0};
	if (getNodeByNumber(0, &dir)){//Hack again
		printf("node_mknod: директории по адресу %s не существует\n", path);
		return -1;
	}
	//Check is file exists
	//Get filenames
	char *buffer;
	buffer = malloc(dir.content_size);
	int length = readContent(&dir, buffer, 0, dir.content_size);
	char *current_fname; char *numText;
	int fname_length = 0;
  printf("node_mknod: проверка, существует ли файл %s\n", filename);
	if (dir.content_size>0)
	while (buffer != NULL) {
	  //Get name length
	  int fs = (int)(strstr(buffer, " ") - buffer);
	  numText = (char *)malloc(fs);
	  strncpy(numText, buffer, fs);
	  //Get file name
	  fname_length = atoi(numText);
	  current_fname = (char *)malloc(fname_length);
	  strncpy(current_fname, buffer+fs+1, fname_length);
	  current_fname[fname_length]=0;
		//Checking
		if (!strcmp(current_fname, filename)){
			return -EEXIST;
		}
	  //Prepare buffer to next iteration
	  buffer = strstr(buffer, "\n");
	  if (buffer != NULL) buffer = buffer + 1;
	}
	//Prepare entry
	buffer = (char *)malloc(100);
  if (dir.content_size>0){
	  sprintf(buffer, "\n%d %s%d", (int)strlen(filename), filename, new_node.number);
  } else {
    sprintf(buffer, "%d %s%d", (int)strlen(filename), filename, new_node.number);
  }
	writeContent(&dir, buffer, dir.content_size, strlen(buffer));
	//Update directory node
	updateNode(dir.number, &dir);
	return res;
}
//Get file information
static int node_getattr(const char *path, struct stat *stbuf){
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
	//Get directory node
	struct my_node dir = (struct my_node){0};
	if (getNodeByNumber(0, &dir)){//A lit hack, while i have only one dir
    printf("node_readdir: директории по адресу %s не существует\n", path);
		return 1;
	}
	//Check is directory empty
	if (dir.content_size == 0){
		printf("node_readdir: директория %s пуста\n", path);
		 return 0;
	 }
	//Get filenames
	char *buffer;
	buffer = malloc(dir.content_size);
	int length = readContent(&dir, buffer, 0, dir.content_size);
	char *current_fname; char *numText;//A lit hack, while i have only one dir
	int fname_length = 0;
	while (buffer != NULL) {
		//Get name length
		int fs = (int)(strstr(buffer, " ") - buffer);
		numText = (char *)malloc(fs);
		strncpy(numText, buffer, fs);
		//Get file name
		fname_length = atoi(numText);
		current_fname = (char *)malloc(fname_length);
		strncpy(current_fname, buffer+fs+1, fname_length);
		current_fname[fname_length]=0;
    //Add to filler
		filler(buf, current_fname, NULL, 0);
		//Prepare buffer to next iteration
		buffer = strstr(buffer, "\n");
		if (buffer != NULL) buffer = buffer + 1;
	}
	return 0;
}

static int node_open(const char *path, struct fuse_file_info *fi)
{
	/*if (strcmp(path, hello_path) != 0)
		return -ENOENT;
*/
  printf("node_open: открытие файла %s, mode %d\n",
	  path,
		fi->flags);
	//Check rights - not now
	/*
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
  */
	return 0;
}
//Read buffer from file
static int node_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	int len;
	printf("nore_read: чтение %d байт из файла %s с позиции %d\n",
	  (int)size,
		path,
		(int)offset);
	//Get node by path
	struct my_node node = (struct my_node){0};
	if (getNodeByPath(path, &node)){
		printf("node_read: не удалось получить информацию о файле %s\n", path);
		return -ENOENT;
	}
	//Read some info from node
	len = readContent(&node, buf, offset, size);
	//Size
	return len;
}
//Write buffer to file
static int node_write(const char *path, const char *buf, size_t size, off_t offset,
         struct fuse_file_info *fi)
{
	printf("node_write: запись %d байт на позицию %d файла %s\n",
    (int)size,
	  (int)offset,
	  path);
	//Get node by path
	struct my_node node = (struct my_node){0};
	if (getNodeByPath(path, &node)){
		printf("node_read: не удалось получить информацию о файле %s\n", path);
		return -ENOENT;
	}
	//Write node content
	int len = 0;
	len = writeContent(&node, buf, offset, size);
	return len;
}
//Operations
static struct fuse_operations hello_oper = {
	.getattr	= node_getattr,
	.readdir	= node_readdir,
	.open		= node_open,
	.read		= node_read,
	.write = node_write,
	.mknod = node_mknod,
	.unlink = node_unlink
};

int main(int argc, char *argv[])
{
	mynode_initialization();
	initializeContent();
	return fuse_main(argc, argv, &hello_oper, NULL);
}
