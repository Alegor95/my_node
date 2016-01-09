#include <time.h>
#include <sys/types.h>
//
#ifndef MYNODE_H_INCLUDED
#define MYNODE_H_INCLUDED
//Inode consts
#define INODE_COUNT 100
#define DIRECT_COUNT 10
#define NODE_DEBUG 0
//Inode structure
typedef struct my_node{
	int number;
	unsigned int mode;
  unsigned int device_id;
	unsigned int owner_id;
	unsigned int owner_group_id;
	time_t c_time;
	time_t m_time;
	time_t a_time;
	unsigned int block_count;
	unsigned int content_size;
	int direct_blocks[DIRECT_COUNT];
	int indirect_block;
} my_node;
//Methods
int addNode(struct my_node *node);
int mynode_initialization();
int getNumberByPath(const char *path);
int getNodeByNumber(int number, struct my_node *node);
int getNodeByPath(const char *path, struct my_node *node);
int updateNode(int number, my_node *node);
int removeNode(int number);
int fillNode(struct my_node *node, int number, int mode, int dev_id);
#endif
