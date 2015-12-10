#include "mynode.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
//mynode defenition and it methods
//Inode consts
#define INODE_COUNT 100
#define DIRECT_COUNT 10
#define BLOCK_SIZE 4096
#define BLOCK_COUNT 512
//Cache of inodes
struct my_node my_nodes_cache[INODE_COUNT];
//Add node to list
int addNode(struct my_node *node){
  return _addNode(node, 1);
}
int _addNode(struct my_node *node, int errorsThrow){
  //Find node number
  unsigned int new_number = 0; //Because 0 is reserved for root mynode
  unsigned int i = 0;
  for (i = 0; i < INODE_COUNT; i++){
    if (my_nodes_cache[i].mode == 0){
      new_number = i;
      break;
    }
  }
  //Free number not found - throw error(if it not internal call)
  if (new_number == 0 && errorsThrow) return -1;
  //Write node to Cache
  memcpy(&my_nodes_cache[new_number], node, sizeof(my_node));
  printf("Добавлена нода с номером %d, mode %d\n",
    new_number,
    my_nodes_cache[new_number].mode);
}
//Fill node with values
int fillNode(struct my_node *node, int mode, int dev_id, int oid, int gid, char *content){
	node->mode = mode;
	node->owner_id = oid;
	node->owner_group_id = gid;
  node->device_id = dev_id;
	time_t now;
	localtime(&now);
	node->c_time = now;
	node->m_time = now;
	node->a_time = now;
	node->block_size = BLOCK_SIZE;
	node->block_count = 0;
	//strcpy(node->content, content);
  return 0;
}
//Get node by it number
int getNodeByNumber(int number, my_node *buffer){
  printf("Попытка получения ноды с номером %d\n", number);
  if (my_nodes_cache[number].mode == 0){
    printf("Нода с номером %d не найдена\n", number);
    return -1;
  }
  memcpy(buffer, &my_nodes_cache[number], sizeof(my_node));
  printf("Получена нода с номером %d, mode %d\n",
    number,
    buffer->mode);
  return 0;
}
//Update node information
int updateNode(int number, my_node *node){
  printf("Попытка обновления ноды с номером %d, mode %d\n",
    number,
    my_nodes_cache[number].mode);
  if (my_nodes_cache[number].mode == 0){
    printf("Нода с номером %d не найдена\n", number);
    return -1;
  }
  memcpy(&my_nodes_cache[number], node, sizeof(my_node));
  printf("Обновлена нода с номером %d, mode %d\n",
    number,
    my_nodes_cache[number].mode);
  return 0;
}
//Remove node
int removeNode(int number){
  my_nodes_cache[number].mode == 0;
  printf("Удалена нода с номером %d\n",
    number);
  return 0;
}
//Initialization of mynode library
int mynode_initialization(){
  printf("Инициализация нод %s\n", "");
  //Clear all mynodes
  unsigned int i = 0;
  for (i = 0; i < INODE_COUNT; i++){
    my_nodes_cache[i] = (struct my_node){0};
  }
  //Add root node (0 number)
  printf("Инициализация корневой ноды %s\n", "");
  struct my_node root = (struct my_node){0};
  fillNode(&root, S_IFDIR | 0755, 0, 0, 0, "shit");//Директория же ж
  _addNode(&root, 0);
  printf("%s\n", "Система нод готова");
}
