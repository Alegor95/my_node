#include "node_content.h"
#include "mynode.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
//Consts
#define CONTENT_DEBUG 1
#define BUFFER_LENGTH 1000
#define INODE_COUNT 100
//Temp content buffer
char temp_content[INODE_COUNT][BUFFER_LENGTH];
//Methods implementation
//Read content from RAM
int readContent(struct my_node *node, char *buffer, off_t offset, size_t length){
  //Just tmp
  if (node->content_size == 0){
    node -> content_size = 11;
  }
  if (offset+length >= node->content_size){
    length = node->content_size-offset;
  }
  if (CONTENT_DEBUG) printf("debug: чтение контента для ноды %d, кол-во байт %d\n",
    node->number,
    (int)length);
  memcpy(buffer, &temp_content[node->number]+offset, length);
  return length;
}
//Write content to RAM
int writeContent(struct my_node *node, char *buffer, off_t offset, size_t length){
  if (CONTENT_DEBUG) printf("debug: запись контента длины %d для ноды %d\n",
    (int)sizeof(buffer),
    node->number);
  node->content_size = length;
  memcpy(&temp_content[node->number], buffer, length);
  return 0;
}
int initializeContent(){
  int i = 0;
  char defaultContent[11] = "holy shiiit";
  for (i = 0; i < INODE_COUNT; i++){
    memcpy(&temp_content[i], defaultContent, 11);
    temp_content[i][12] = 0;
  }
}
