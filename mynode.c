#include "mynode.h"
#include "file_service.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
//mynode defenition and it methods
//Cache of inodes
struct my_node my_nodes_cache[INODE_COUNT];
//Add node to list
int addNode(struct my_node *node){
  return _addNode(node, 1);
}
int _addNode(struct my_node *node, int errorsThrow){
  //Add node to file_service
  int errorCode = addNodeToFile(node);
  if (errorCode){
    return errorCode;
  }
  //Everything ok
  if (NODE_DEBUG) printf("debug: добавлена нода с номером %d\n",
    node->number);
  return 0;
}
//Fill node with values
int fillNode(struct my_node *node, int node_n, int mode, int dev_id){
  node->number = node_n;
	node->mode = mode;
  //
  int oid = (int)getuid();
	node->owner_id = oid;
  int gid = (int)getgid();
	node->owner_group_id = gid;
  node->device_id = dev_id;
	time_t now;
	now = time(0);
	node->c_time = now;
	node->m_time = now;
	node->a_time = now;
	node->block_count = 0;
	node->content_size = 0;
  return 0;
}
//Get node by it number
int getNodeByNumber(int number, my_node *buffer){
  if (NODE_DEBUG) printf("debug: попытка получения ноды с номером %d\n", number);
  if (readNodeFromFile(number, buffer)){
    if (NODE_DEBUG) printf("debug: нода с номером %d не найдена\n", number);
    return -1;
  }
  if (NODE_DEBUG) printf("debug: получена нода с номером %d, контент %d\n",
    number,
    buffer->content_size);
  return 0;
}
//Получение нода файла в директории
int getNumberFromDirectory(struct my_node *dir, const char *filename){
  if (NODE_DEBUG) printf("debug: получение номера ноды файла %s директории %d\n",
    filename,
    dir->number);
  if (strlen(filename) == 0){
    if (NODE_DEBUG) printf("debug: имя файла пустое - возвращаем директорию%s\n", "");
    return dir->number;
  }
  if (dir->content_size == 0){
    return -1;
  }
  char *buffer;
  buffer = (char *)malloc(dir->content_size);
  readContentFromFile(dir, buffer, 0, dir->content_size);
  char *current_fname; char *numText;
  int fname_length = 0; int node_num = -1;
  while (buffer != NULL) {
    //Parse directory entry
  		//Get name length
      char *space_pos = strstr(buffer, " ");
      if (space_pos == NULL) break;
  		int fs = (int)(space_pos - buffer);
  		numText = (char *)malloc(fs);
  		strncpy(numText, buffer, fs);
  		fname_length = atoi(numText);
      free(numText);
  		//Get file name
  		current_fname = (char *)malloc(fname_length);
  		strncpy(current_fname, buffer+fs+1, fname_length);
  		current_fname[fname_length]=0;
      char *next_line = strstr(buffer, "\n");
      if (NODE_DEBUG) printf("debug: проверка имени %s %d\n", current_fname, fname_length);
      if (strlen(current_fname)) //if current_fname not empty
      if (!strcmp(current_fname, filename)){
        if (NODE_DEBUG) printf("debug: имя %s %d подходит\n", current_fname, fname_length);
        //Found, try to get node
        char *node_text;
        int node_len;
        if (next_line != NULL){
          node_len = (int)(next_line-buffer) - fs -1 - fname_length;
        } else {
          node_len=(int)(strlen(buffer) -fs - 1 - fname_length);
        }
        node_text = (char *)malloc(node_len);
        strncpy(node_text, buffer+fs+1+fname_length, node_len);
        if (NODE_DEBUG) printf("debug: id ноды %s\n", node_text);
        node_num = atoi(node_text);
        free(node_text);
        free(current_fname);
        break;
      } else {
        if (NODE_DEBUG) printf("debug: имя %s %d не подходит\n", current_fname, fname_length);
        free(current_fname);
      }
    //Prepare buffer to next iteration
    buffer = next_line;
    if (buffer != NULL) buffer = buffer + 1;
  }
  return node_num;
}
//Вычисление ноды из пути
int getNumberByPath(const char *path){
  if (NODE_DEBUG) printf("debug: вычисление номера ноды из пути %s\n", path);
  //Skip first slash - it's a root dir
  path = strstr(path, "/")+1;
  struct my_node dir;
  getNodeByNumber(0, &dir);
  while (path != NULL) {
    //Get directory
    char *next_path = strstr(path, "/");
    if (next_path != NULL) {
      printf("%s\n", path);
      printf("%s\n", next_path);
      printf("%d\n", (int)(next_path-path));
      //If not end of path
      //Change dir
      int dir_name_l = (int)(next_path-path);
      char *dir_name = (char *)malloc(dir_name_l);
      printf("%d\n", dir_name_l);
      strncpy(dir_name, path, dir_name_l);
      dir_name[dir_name_l]=0;
      if (NODE_DEBUG) printf("debug: следующая директория %s\n", dir_name);
      int dir_num = getNumberFromDirectory(&dir, dir_name);
      getNodeByNumber(dir_num, &dir);
      //Prepare next iteration
      path = next_path + 1;
      free(dir_name);
    } else {
      if (NODE_DEBUG) printf("debug: конечная нода с именем \"%s\"\n", path);
      //If it's last entry, return number
      int number = getNumberFromDirectory(&dir, path);
      return number;
    }
  }
  //Nothing found, return -1
  return -1;
}
//Get node by it path
int getNodeByPath(const char *path, struct my_node *node){
  int number = getNumberByPath(path);
  if (number < 0 || getNodeByNumber(number, node)){
    if (NODE_DEBUG) printf("debug: нода с путем %s не найдена\n", path);
    return -1;
  }
  return 0;
}
//Update node information
int updateNode(int number, my_node *node){
  if (NODE_DEBUG) printf("debug: попытка обновления ноды с номером %d, mode %d\n",
    number,
    my_nodes_cache[number].mode);
  if (writeNodeToFile(number, node)){
    if (NODE_DEBUG) printf("debug: нода с номером %d не найдена\n", number);
    return -1;
  }
  if (NODE_DEBUG) printf("debug: обновлена нода с номером %d, mode %d\n",
    number,
    my_nodes_cache[number].mode);
  return 0;
}
//Remove node
int removeNode(int number){
  removeNodeFromFile(number);
  if (NODE_DEBUG) printf("debug: удалена нода с номером %d\n",
    number);
  return 0;
}
//Initialization of mynode library
int mynode_initialization(){
  if (NODE_DEBUG) printf("debug: инициализация нод %s\n", "");
  //Clear all mynodes
  unsigned int i = 0;
  for (i = 0; i < INODE_COUNT; i++){
    my_nodes_cache[i] = (struct my_node){0};
  }
  //Add root node (0 number)
  if (getFreeNodeNum() == 0){
    if (NODE_DEBUG) printf("debug: инициализация корневой ноды %s\n", "");
    struct my_node root = (struct my_node){0};
    fillNode(&root, 0, S_IFDIR | 0755, 0);//Директория же ж
    _addNode(&root, 0);
  }
  if (NODE_DEBUG) printf("%s\n", "debug: система нод готова");
}
