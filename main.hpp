#pragma once

#include <cassert>

#include "params.h"

#define VISITED 1
#define NOT_VISITED 0

typedef struct Node {
    char header;
    struct Node* refs[REFS_IN_NODE];
} Node;

#define NODES_N GRAPH_SIZE_GB * 1024 * 1024 * 1024 / sizeof(Node)

char nodeIsVisited(Node* node);