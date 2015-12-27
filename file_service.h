#include "mynode.h"
#ifndef FILE_SERVICE_H_INCLUDED
#define FILE_SERVICE_H_INCLUDED
#define NODE_SIZE sizeof(struct my_node)
#define FILE_PATH "fs_iso"
#define BLOCK_COUNT 1024
#define BLOCK_SIZE 1024
#define NODE_META_SIZE sizeof(int)*INODE_COUNT
#define BLOCK_META_SIZE sizeof(int)*BLOCK_COUNT
#define FILE_DEBUG 1
int readNodeFromFile(int num,struct my_node *node);
int writeNodeToFile(int num, struct my_node *node);
int addNodeToFile(struct my_node *node);
int removeNodeToFile(int num);
int initializeFile();
int destroyFile();
#endif
