#include <time.h>
#ifndef MYNODE_H_INCLUDED
#define MYNODE_H_INCLUDED
//Inode structure
typedef struct my_node{
	unsigned int mode;
  unsigned int device_id;
	unsigned int owner_id;
	unsigned int owner_group_id;
	time_t c_time;
	time_t m_time;
	time_t a_time;
	unsigned int block_size;
	unsigned int block_count;
	//char *content;
} my_node;
//Methods
int addNode(struct my_node *node);
int mynode_initialization();
int getNodeByNumber(int number, struct my_node *node);
int updateNode(int number, my_node *node);
int removeNode(int number);
int fillNode(struct my_node *node, int mode, int dev_id, int oid, int gid, char *content);
#endif
