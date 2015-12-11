#include "mynode.h"
#include <sys/types.h>
#ifndef NODE_CONTENT_H_INCLUDED
#define NODE_CONTENT_H_INCLUDED
int readContent(struct my_node *node, char *buffer, off_t offset, size_t length);
int writeContent(struct my_node *node, const char *buffer, off_t offset, size_t length);
int initializeContent();
#endif
