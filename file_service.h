#include "mynode.h"
#include <sys/types.h>
#ifndef FILE_SERVICE_H_INCLUDED
#define FILE_SERVICE_H_INCLUDED
//Constants
#define NODE_SIZE sizeof(struct my_node)
#define FILE_PATH "fs_iso"
#define BLOCK_COUNT 1024
#define BLOCK_SIZE 16
#define NODE_META_SIZE sizeof(int)*INODE_COUNT
#define BLOCK_META_SIZE sizeof(int)*BLOCK_COUNT
#define FILE_DEBUG 1
//Node methods
int readNodeFromFile(int num,struct my_node *node);
int writeNodeToFile(int num, struct my_node *node);
int addNodeToFile(struct my_node *node);
int removeNodeFromFile(int num);
//Content methods
int readContentFromFile(struct my_node *node, char *buffer,
  off_t offset, size_t length);
int writeContentToFile(struct my_node *node, const char *buffer,
  off_t offset, size_t length);
//File service methods
int initializeFile();
int destroyFile();
#endif
