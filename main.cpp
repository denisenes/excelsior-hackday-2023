#include <malloc.h>
#include <cassert>
#include <stdio.h>
#include <sys/time.h>

#include "main.hpp"
#include "traversal.hpp"

Node* graph;

unsigned long long edge_counter;

Node * allocateSpaceForGraph() {
    return (Node*) calloc(NODES_N, sizeof(Node));
}

void setRandomEdges() {
    for (int i = 0; i < NODES_N; i++) {
        assert(graph[i].header == NOT_VISITED);

        for (int j = 0; j < REFS_IN_NODE; j++) {
            char isNull = ((unsigned int) rand()) % SPARCITY_PARAM != 0;

            if (isNull) {
                continue;
            }

            int idx = NODES_N;
            Node* child = &graph[rand() % idx];
            (&graph[i])->refs[j] = child;
            edge_counter++;
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
    
}

int main(int argc, char ** argv) {
    Traversal* traversal_algo = new PrefetchBufferOnPop();

    printf("\nInitalization...\n");
    init();

    printf("Setting edges...\n");
    setRandomEdges();
    printf("Number of nodes: %llu\n", NODES_N);
    printf("Number of edges: %llu\n", edge_counter);

    printf("Collecting roots...\n");
    traversal_algo->collectRoots(graph);

    printf("\nDFS started\n");
    
    long long startTime = currentTimeMillis();
    traversal_algo->traverseGraph();
    long long endTime   = currentTimeMillis();

    printf("DFS ended\n");

    int totalVisited = countVisited();

    printf("\n==============\n");
    printf("Total visited (Nodes): %d\n", totalVisited);
    printf("Total visited (%%): %f\n", totalVisited / ((double) (NODES_N)) * 100);
    printf("Total visited (Mb): %lld\n", ((long long) sizeof(Node) * totalVisited / 1024 / 1024)); // used for seed tuning
    printf("Traversal time: %lld\n", endTime - startTime);

    return 0;
}