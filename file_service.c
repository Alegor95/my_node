#include "file_service.h"
#include <stdio.h>
#include <errno.h>
//File
FILE *file;
//Meta information about node and data blocks filling
int mynode_fill[INODE_COUNT];
//Count node filled position in file
int getNodeFillFilePosition(int num){
  return num*sizeof(int);
}
//Count node position in file
int countNodeFilePosition(int num){
  return NODE_META_SIZE+BLOCK_META_SIZE+NODE_SIZE*num;
}
//Read node from file
int readNodeFromFile(int num, struct my_node *node){
  printf("file_service: чтение ноды с номером %d в файл\n", num);
  if (!mynode_fill[num]){
    if (FILE_DEBUG) printf("debug: нода %d не найдена в файле\n", num);
    return -EBADF;
  }
  int file_p = countNodeFilePosition(num);
  fseek(file, file_p, SEEK_SET);
  fread(node, NODE_SIZE, 1, file);
  return 0;
}
//Write node to file
int writeNodeToFile(int num, struct my_node *node){
  if (!mynode_fill[num]){
    if (FILE_DEBUG) printf("debug: нода %d записывается в файл заного\n", num);
    mynode_fill[num] = 1;
    //write to file
    fseek(file, getNodeFillFilePosition(num), SEEK_SET);
    fwrite(&mynode_fill[num], sizeof(int), 1, file);
  }
  printf("file_service: запись ноды с номером %d в файл\n", num);
  int file_p = countNodeFilePosition(num);
  fseek(file, file_p, SEEK_SET);
  fwrite(node, NODE_SIZE, 1, file);
  return 0;
}

//Add node to file
int addNodeToFile(struct my_node *node){
  printf("file_service: создание ноды в файле%s\n", "");
  int node_num = getFreeNodeNum();
  if (node_num < 0){
    return node_num;
  }
  writeNodeToFile(node_num, node);
  return 0;
}
int removeNodeToFile(int num){
  if (mynode_fill[num]){
    if (FILE_DEBUG) printf("debug: нода %d удаляется из файла\n", num);
    mynode_fill[num] = 0;
    //write to file
    fseek(file, getNodeFillFilePosition(num), SEEK_SET);
    fwrite(&mynode_fill[num], sizeof(int), 1, file);
  }
}
int getFreeNodeNum(){
  int i = 0;
  for (i = 0; i < INODE_COUNT; i++){
    if (!mynode_fill[i]){
      if (FILE_DEBUG) printf("debug: найдена свободная нода %d\n", i);
      return i;
    }
  }
  if (FILE_DEBUG) printf("debug: не найдено свободных нод%s\n", "");
  return -ENFILE;
}
//File system Initialization
int initializeFile(){
  file = fopen(FILE_PATH, "r+");
  if (file == NULL){
    printf("file_service: Не удалось открыть образ %s\n", FILE_PATH);
    return 1;
  }
  //Set file size
  if (!ftruncate(file, NODE_META_SIZE+BLOCK_META_SIZE
    +INODE_COUNT*NODE_SIZE
    +BLOCK_COUNT*BLOCK_SIZE)){

  }
  int i = 0;
  //Read meta information
  //nodes
  for(i = 0; i < INODE_COUNT; i++){
    int is_filled = 0;
    fseek(file, getNodeFillFilePosition(i), SEEK_SET);
    fread(&is_filled, sizeof(int), 1, file);
    if (is_filled != 1) is_filled = 0;
    mynode_fill[i] = is_filled;
  }
  if (FILE_DEBUG){
    //debug node output
    for (i = 0; i < INODE_COUNT; i++){
      if (mynode_fill[i]) printf("debug: node %d readed\n", i);
    }
  }
  //data blocks
  return 0;
}
//Destroy file system
int destroyFile(){
  //flush meta Cache
  int i = 0;
  //nodes
  for(i = 0; i < INODE_COUNT; i++){
    int is_filled = 0;
    fseek(file, getNodeFillFilePosition(i), SEEK_SET);
    fwrite(&mynode_fill[i], sizeof(int), 1, file);
  }
  //finalize
  printf("file_service: закрытие образа %s\n", FILE_PATH);
  fclose(file);
  return 0;
}
