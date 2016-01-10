#include "file_service.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
//File
FILE *file;
//Meta information about node and data blocks filling
int mynode_fill[INODE_COUNT];
int block_fill[BLOCK_COUNT];
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
  printf("file_service: чтение ноды с номером %d из файла\n", num);
  if (num<0 && num>=INODE_COUNT && !mynode_fill[num]){
    if (FILE_DEBUG) printf("debug: нода %d не найдена в файле\n", num);
    return -EBADF;
  }
  int file_p = countNodeFilePosition(num);
  //Clear node
  memset(node, 0, NODE_SIZE);
  //
  fseek(file, file_p, SEEK_SET);
  fread(node, NODE_SIZE, 1, file);
  printf("file_service: нода %d успешно получена из файла, длина контента %d\n",
    node->number, node->content_size);
  return 0;
}
//Write node to file
int writeNodeToFile(int num, struct my_node *node){
  printf("file_service: запись ноды с номером %d в файл\n", num);
  if (num<0 && num>=INODE_COUNT){
    if (FILE_DEBUG) printf("debug: некорректный номер ноды %d\n", num);
    return -EBADF;
  }
  if (!mynode_fill[num]){
    if (FILE_DEBUG) printf("debug: нода %d записывается в файл заного\n", num);
    mynode_fill[num] = 1;
    //write to file
    fseek(file, getNodeFillFilePosition(num), SEEK_SET);
    fwrite(&mynode_fill[num], sizeof(int), 1, file);
  }
  int file_p = countNodeFilePosition(num);
  fseek(file, file_p, SEEK_SET);
  fwrite(node, NODE_SIZE, 1, file);
  fflush(file);
  return 0;
}

//Add node to file
int addNodeToFile(struct my_node *node){
  printf("file_service: создание ноды в файле%s\n", "");
  int node_num = getFreeNodeNum();
  if (node_num < 0){
    return node_num;
  }
  node->number = node_num;
  writeNodeToFile(node_num, node);
  return 0;
}
int removeNodeFromFile(int num){
  if (mynode_fill[num]){
    if (FILE_DEBUG) printf("debug: нода %d удаляется из файла\n", num);
    mynode_fill[num] = 0;
    //write to file
    fseek(file, getNodeFillFilePosition(num), SEEK_SET);
    fwrite(&mynode_fill[num], sizeof(int), 1, file);
    fflush(file);
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
//File block methods
long int getBlockPosition(int block_num){
  return NODE_META_SIZE+BLOCK_META_SIZE
    +NODE_SIZE*INODE_COUNT
    +block_num*BLOCK_SIZE;
}
long int getBlockFillPosition(int block_num){
  return NODE_META_SIZE+sizeof(int)*block_num;
}
int getBlockNumberFromNode(struct my_node *node, int num){
  int block_num = -1;
  if (FILE_DEBUG)
    printf("debug: вычисление номера блока данных %d из ноды %d\n",
      num, node -> number);
  if (num < node->block_count){
    if (num < DIRECT_COUNT){
      //We have direct link to data
      block_num = node -> direct_blocks[num];
    } else {
      //We have undirect link, try get it number
      char *read_buffer;
      read_buffer = (char *)malloc(BLOCK_SIZE);
      //Read block with position
      readBlockFromFile(node->indirect_block, read_buffer, 0, BLOCK_SIZE);
      char *buffer = read_buffer;
      //Get node number
      char *pos;
      char *next_pos;
      int i = DIRECT_COUNT;
      do {
        pos = (char *)strstr(buffer, "\n");
        if (pos != NULL){
          pos += sizeof(char);
          if (i == num  && strlen(pos) != 0){
            next_pos = (char *)strstr(pos, "\n");
            if (next_pos == NULL) break;
            int block_num_l = next_pos - pos;
            char *block_num_text = (char *)malloc(block_num_l+1);
            strncpy(block_num_text, pos, block_num_l);
            block_num_text[block_num_l] = 0;
            block_num = atoi(block_num_text);
            free(block_num_text);
            break;
          }
        }
        buffer = pos;
        i++;
      } while (pos != NULL);
      free(read_buffer);
    }
  }
  if (block_num < -1){
    if (FILE_DEBUG) printf("debug: ошибка при вычислении блока %d\n", num);
    return -EFAULT;
  } else {
    if (FILE_DEBUG) printf("debug: успешно вычислен номер блока %d\n",
      block_num);
    return block_num;
  }
}
//Read Content from file
int readContentFromFile(struct my_node *node, char *buffer,
  off_t offset, size_t length){
  //Block reading
  printf("file_service: чтение контента ноды %d, кол-во байт %d\n",
    node->number, (int)length);
  //
  if (length>node->content_size) length=node->content_size;
  //Get first block
  int now_block = offset / BLOCK_SIZE;
  offset = offset % BLOCK_SIZE;
  //Read, while readed < length
  int readed = 0;
  while (readed < length) {
    //Count data position and length
    int block_num = getBlockNumberFromNode(node, now_block);
    int buffer_position = now_block*BLOCK_SIZE;
    int read_length = BLOCK_SIZE-offset;
    if (length - readed < BLOCK_SIZE-offset) read_length = length - readed;
    //Read block to buffer
    if (readBlockFromFile(block_num, buffer+buffer_position,
      offset, read_length)){
        return -EIO;
      }
    //Prepare next iteration
    if (readed == 0){
      readed += BLOCK_SIZE - offset;
      offset = 0;
    } else {
      readed += read_length;
    }
    now_block++;
    printf("readed %d/%d\n", readed, (int)length);
  }
  return length;
}

int readBlockFromFile(int block_num, char *buffer, off_t offset, size_t length){
  if (FILE_DEBUG) printf("debug: считывание блока %d из файла\n", block_num);
  if (!block_fill[block_num]) {
    printf("file_service: блок %d не записан, ошибка чтения\n", block_num);
    return -EIO;
  }
  long int block_position = getBlockPosition(block_num);
  if (offset >= BLOCK_SIZE) {
    if (FILE_DEBUG) printf("debug: некорректный сдвиг %d\n", (int)offset);
    return -EIO;
  }
  block_position+=(int)offset;
  fseek(file, block_position, SEEK_SET);
  //Count length of read interval
  if (offset + length > BLOCK_SIZE) {
    if (FILE_DEBUG) printf("debug: некорректная длина %d\n", (int)length);
    return -EIO;
  }
  if (FILE_DEBUG) printf("debug: чтение с позиции %ld, длина контента %d\n",
    block_position, (int)length);
  fread(buffer, sizeof(char), length, file);
  printf("буфер %s\n", buffer);
  return 0;
}
//Write buffer to file, with length and offset
int writeContentToFile(struct my_node *node, const char *buffer,
  off_t offset, size_t length){
  //Write content to file
  printf("file_service: запись контента для ноды %d в файл, длина %d поз %d\n",
    node->number, (int)length, (int)offset);
  //Count new file size
  if (offset+length>node->content_size){
    node->content_size=offset+length;
  }
  //truncate node
  truncateNodeBlocks(node);
  //fill file blocks from buffer
  int written_bytes=0;
  int start_block=offset/BLOCK_SIZE;
  offset=offset%BLOCK_SIZE;
  while (written_bytes < length) {
    //count parameters
    int current_offset = offset;
    if (written_bytes == 0) offset = 0;
    int current_length = BLOCK_SIZE - current_offset;
    if (length - written_bytes<BLOCK_SIZE-current_offset)
      current_length = length-written_bytes;
    int current_block_num = getBlockNumberFromNode(node, start_block);
    //Write buffer to file
    writeBlockToFile(current_block_num, buffer+written_bytes,
      current_offset, current_length);
    //Done, next iteration
    start_block++;
    written_bytes+=current_length;
  }
  return length;
}
//Write block to file
int writeBlockToFile(int block_num, char *buffer, off_t offset, size_t length){
  if (FILE_DEBUG) printf("debug: запись блока %d в файл\n", block_num);
  long int block_position = getBlockPosition(block_num);
  if (offset >= BLOCK_SIZE) {
    if (FILE_DEBUG) printf("debug: некорректный сдвиг %d\n", (int)offset);
    return -EIO;
  }
  block_position+=(int)offset;
  fseek(file, block_position, SEEK_SET);
  //Count length of read interval
  if (offset + length > BLOCK_SIZE) {
    if (FILE_DEBUG) printf("debug: некорректная длина %d\n", (int)length);
    return -EIO;
  }
  if (FILE_DEBUG) printf("debug: запись на позицию %ld, длина %d, %.*s\n",
    block_position, (int)length, (int)length, buffer);
  fwrite(buffer, sizeof(char), length, file);
  //Set block as filled
  if (!block_fill[block_num]){
    //Update cache
    block_fill[block_num] = 1;
    //Write to file
    int block_fill_position = getBlockFillPosition(block_num);
    fseek(file, block_fill_position, SEEK_SET);
    fwrite(&block_fill[block_num], sizeof(int), 1, file);
  }
  fflush(file);
  return 0;
}
//Truncate file blocks to required content size
int truncateNodeBlocks(struct my_node *node){
  printf("file_service: проверка количества блоков для ноды %d\n",
    node->number);
  int avaliable_size = node->block_count*BLOCK_SIZE;
  int used_size = node->content_size;
  //Check is new blocks need
  if (used_size>avaliable_size){
    printf("file_service: ноде %d выделено недостаточно блоков\n",
      node->number);
    //Count nessecary block count
    int new_block_count = used_size / BLOCK_SIZE;
    if (used_size % BLOCK_SIZE > 0) new_block_count++;
    int blocks_to_add = new_block_count - node->block_count;
    //Add new blocks to node
    int nb = 0;
    char *indirect_buffer = NULL;
    for (nb = 0; nb < blocks_to_add; nb++){
      //Get free block number
      int new_block_num = getFreeBlockNum();
      //Error thrown
      if (getFreeBlockNum < 0) return new_block_num;
      //Prepare new block
      prepareNewBlock(new_block_num);
      if (FILE_DEBUG) printf("debug: подготовлен новый блок %d\n",
        new_block_num);
      //Write link to new block
      if (node->block_count < DIRECT_COUNT){
        //Just add block num to direct
        node->direct_blocks[node->block_count] = new_block_num;
      } else {
        if (node->block_count == DIRECT_COUNT){
          //Prepare block for indirect blocks
          int new_indirect_block = getFreeBlockNum();
          prepareNewBlock(new_indirect_block);
          node->indirect_block = new_indirect_block;
          if (FILE_DEBUG) printf("debug: добавлен блок со ссылками %d\n",
            new_indirect_block);
        }
        //Write new block
        if (indirect_buffer == NULL){
          indirect_buffer = (char *)malloc(BLOCK_SIZE);
          readBlockFromFile(node->indirect_block, indirect_buffer,
            0, BLOCK_SIZE);
        }
        if (FILE_DEBUG) printf("debug: добавление ссылки на блок %d\n",
          new_block_num);
        //Add new entry to indirect block
        char *entry = (char *)malloc(sizeof(int)*8+1);
        sprintf(entry, "%d\n", new_block_num);
        char *tmp_block_content = malloc(strlen(indirect_buffer)
          +strlen(entry)+1);
        strcpy(tmp_block_content, indirect_buffer);
        strcat(tmp_block_content, entry);
        free(indirect_buffer);
        indirect_buffer = tmp_block_content;
        free(entry);
      }
      //Add new block
      node->block_count++;
    }
    //Save inderect block
    if (indirect_buffer != NULL){
      writeBlockToFile(node->indirect_block, indirect_buffer, 0, BLOCK_SIZE);
    }
  }
  if (avaliable_size>used_size){
    //Checking: is file use so many blocks
    int required_blocks = used_size/BLOCK_SIZE;
    if (used_size%BLOCK_SIZE != 0) required_blocks++;
    if (required_blocks < node->block_count){
      printf("file_service: ноде %d выделено слишком много блоков\n",
        node->number);
      //Deattach unused blocks from file
      int unused_count = node->block_count - required_blocks;
      int ub = 0;
      for (ub = 0; ub < unused_count; ub++){
        //Remove block from node
        //Remove block from file
        int unused_block_num = getBlockNumberFromNode(node, node->block_count-1);
        clearBlockInFile(unused_block_num);
        //Last iteration
        node->block_count--;
      }
    }
  }
  //Check is
  //Finish truncate - save node info
  writeNodeToFile(node->number, node);
}
//Prepare new block
int prepareNewBlock(int block_num){
  //Fill block with 0 and mark as filled
  char *buffer = (char *)malloc(BLOCK_SIZE);
  memset(buffer, 0, BLOCK_SIZE);
  writeBlockToFile(block_num, buffer, 0, BLOCK_SIZE);
  free(buffer);
}
//Get number of free file block
int getFreeBlockNum(){
  int i = 0;
  for (i = 0; i < BLOCK_COUNT; i++){
    if (!block_fill[i]) return i;
  }
  return -ENFILE;
}
//Remove block
int clearBlockInFile(int block_num){
  printf("file_service: очистка блока %d\n", block_num);
  //Update cache
  block_fill[block_num] = 0;
  //Write to file
  int block_fill_position = getBlockFillPosition(block_num);
  fseek(file, block_fill_position, SEEK_SET);
  fwrite(&block_fill[block_num], sizeof(int), 1, file);
  fflush(file);
}
//File system Initialization
int initializeFile(){
  file = fopen(FILE_PATH, "r+");
  if (file == NULL){
    printf("file_service: Не удалось открыть образ %s\n", FILE_PATH);
    return -1;
  }
  //Set file size
  if (!ftruncate(file, NODE_META_SIZE+BLOCK_META_SIZE
    +INODE_COUNT*NODE_SIZE
    +BLOCK_COUNT*BLOCK_SIZE)){
    printf("file_service: ошибка задания размера файла %s\n", FILE_PATH);
    return -1;
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
  //Blocks
  for (i = 0; i < BLOCK_COUNT; i++){
    int is_filled = 0;
    fseek(file, getBlockFillPosition(i), SEEK_SET);
    fread(&is_filled, sizeof(int), 1, file);
    if (is_filled != 1) is_filled = 0;
    block_fill[i] = is_filled;
  }
  //
  if (FILE_DEBUG){
    //debug node output
    for (i = 0; i < INODE_COUNT; i++){
      if (mynode_fill[i]) printf("debug: нода %d заполнена\n", i);
    }
    for (i = 0; i < BLOCK_COUNT; i++){
      if (block_fill[i]) printf("debug: блок %d заполнен\n", i);
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
    fseek(file, getNodeFillFilePosition(i), SEEK_SET);
    fwrite(&mynode_fill[i], sizeof(int), 1, file);
  }
  //blocks
  for (i=0;i<BLOCK_COUNT;i++){
    fseek(file, getBlockFillPosition(i), SEEK_SET);
    fwrite(&block_fill[i], sizeof(int), 1, file);
  }
  //finalize
  printf("file_service: закрытие образа %s\n", FILE_PATH);
  fflush(file);
  fclose(file);
  return 0;
}
