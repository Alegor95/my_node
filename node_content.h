#include "mynode.h"
#include <sys/types.h>
#ifndef NODE_CONTENT_H_INCLUDED
#define NODE_CONTENT_H_INCLUDED
//Consts
#define CONTENT_DEBUG 1
#define BUFFER_LENGTH 1000
int readContent(struct my_node *node, char *buffer, off_t offset, size_t length);
int writeContent(struct my_node *node, const char *buffer, off_t offset, size_t length);
int initializeContent();
#endif
