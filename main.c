#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <x86intrin.h>

#include "main.h"

Node* graph;
Stack traversalStack;

__attribute__((always_inline))
char nodeIsVisited(Node* node) {
    return node->header == VISITED;
}

__attribute__((always_inline))
void stackPush(Node* node) {
    assert(traversalStack.sp < TRAVERSAL_STACK_SIZE);

    traversalStack.buffer[traversalStack.sp] = node;
    traversalStack.sp++;
}

__attribute__((always_inline))
Node* stackPop() {
    assert(traversalStack.sp > 0);

    return traversalStack.buffer[--traversalStack.sp];
}

__attribute__((always_inline))
char stackIsEmpty() {
    return traversalStack.sp == 0;
}

Node * allocateSpaceForGraph() {
    return (Node*) calloc(NODES_N, sizeof(Node));
}

void setRandomEdges() {
    int rootCounter = 0;

    for (int i = 0; i < NODES_N; i++) {
        assert(graph[i].header == NOT_VISITED);

        unsigned char isRoot = ((unsigned int) rand()) % 2;

        for (int j = 0; j < REFS_IN_NODE; j++) {
            char isNull = ((unsigned int) rand()) % SPARCITY_PARAM != 0;

            if (isNull) {
                continue;
            }

            int idx = NODES_N;
            Node* child = &graph[rand() % idx];
            (&graph[i])->refs[j] = child;
        }

        if (isRoot && rootCounter < ROOTS_N) {
            graph[i].header = VISITED;
            stackPush(&graph[i]);
            rootCounter++;
        }
    }
}

void dfs() {
    while (!stackIsEmpty()) {
        Node* parent = stackPop();

        assert(nodeIsVisited(parent));

        for (int i = 0; i < REFS_IN_NODE; i++) {
            Node* child = parent->refs[i];

            if (child != NULL && !nodeIsVisited(child)) {
                child->header = VISITED;
                stackPush(child);
            }
        }
    }
}

int countVisited() {
    int visited = 0;

    for (int i = 0; i < NODES_N; i++) {
        Node* node = &graph[i];

        if (nodeIsVisited(node)) {
            visited++;
        }
    }
    return visited;
}

long long currentTimeMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    long long m1 = (long long) (time.tv_sec) * 1000;
    long long m2 = (long long) (time.tv_usec) / 1000;
    return m1 + m2;
}

void init() {
    srand(SEED);
    graph = allocateSpaceForGraph();
    assert(traversalStack.sp == 0);
}

int main(int argc, char ** argv) {
    printf("Initalization...\n");
    printf("Number of nodes: %llu\n", NODES_N);
    init();

    printf("Setting edges...\n");
    setRandomEdges();

    printf("DFS started\n");
    
    long long startTime = currentTimeMillis();
    dfs();
    long long endTime   = currentTimeMillis();

    printf("DFS ended\n");

    int totalVisited = countVisited();

    printf("\n==============\n");
    printf("Total visited (Nodes): %d\n", totalVisited);
    printf("Total visited (Mb): %lld\n", ((long long) sizeof(Node) * totalVisited / 1024 / 1024)); // used for seed tuning
    printf("Traversal time: %lld\n", endTime - startTime);

    return 0;
}