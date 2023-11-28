#include <cassert>

#include "traversal.hpp"

__attribute__((always_inline))
char nodeIsVisited(Node* node) {
    return node->header == VISITED;
}

void collectsRootsWithoutMarking(Node* graph, NodeStack& stack) {
    int rootCounter = 0;

    for (int i = 0; i < NODES_N; i++) {
        assert(graph[i].header == NOT_VISITED);

        unsigned char isRoot = ((unsigned int) rand()) % 2;
        if (isRoot && rootCounter < ROOTS_N) {
            stack.push(&graph[i]);
            rootCounter++;
        }
    }
}

void collectRootsWithMarking(Node* graph, NodeStack& stack) {
    int rootCounter = 0;

    for (int i = 0; i < NODES_N; i++) {
        assert(graph[i].header == NOT_VISITED);

        unsigned char isRoot = ((unsigned int) rand()) % 2;
        if (isRoot && rootCounter < ROOTS_N) {
            graph[i].header = VISITED;
            stack.push(&graph[i]);
            rootCounter++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

NodeStack BaselineOnPush::stack = NodeStack();

void BaselineOnPush::traverseGraph() {
    while (!stack.isEmpty()) {
        Node* parent = stack.pop();

        for (int i = 0; i < REFS_IN_NODE; i++) {
            Node* child = parent->refs[i];

            if (child != nullptr && !nodeIsVisited(child)) {
                child->header = VISITED; // Prefetch on grey in fact
                stack.push(child);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

NodeStack BaselineOnPop::stack = NodeStack();

void BaselineOnPop::traverseGraph() {
    while (!stack.isEmpty()) {
        Node* parent = stack.pop();

        if (!nodeIsVisited(parent)) {
            parent->header = VISITED;

            for (int i = 0; i < REFS_IN_NODE; i++) {
                Node* child = parent->refs[i];

                if (child != nullptr) stack.push(child);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

NodeStack PrefetchBufferOnPop::stack  = NodeStack();
PrefetchBuffer PrefetchBufferOnPop::pb = PrefetchBuffer();

void PrefetchBufferOnPop::traverseGraph() {
    while (true) {
        while (!stack.isEmpty()) {
            if (pb.isFull()) {
                Node* b = pb.dequeue();
                if (!nodeIsVisited(b)) {
                    b->header = VISITED;
                    for (int i = 0; i < REFS_IN_NODE; i++) {
                        Node* p = b->refs[i];
                        if (p != nullptr) stack.push(p);
                    }
                }
            }
            Node* g = stack.pop();
            pb.enqueueAndPrefetch(g);
        }
        if (pb.isEmpty()) {
            break;
        }
        Node* b = pb.dequeue();
        if (!nodeIsVisited(b)) {
            b->header = VISITED;
            for (int i = 0; i < REFS_IN_NODE; i++) {
                Node* p = b->refs[i];
                if (p != nullptr) stack.push(p);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

NodeStack PrefetchBufferOnPush::stack  = NodeStack();
PrefetchBuffer PrefetchBufferOnPush::pb = PrefetchBuffer();

void PrefetchBufferOnPush::traverseGraph() {
    while (true) {
        while (!stack.isEmpty()) {
            if (pb.isFull()) {
                Node* b = pb.dequeue();
                for (int i = 0; i < REFS_IN_NODE; i++) {
                    Node* p = b->refs[i];
                    if (p != nullptr && !nodeIsVisited(p)) { p->header = VISITED; stack.push(p); }
                }
            }
            Node* g = stack.pop();
            pb.enqueueAndPrefetch(g);
        }
        if (pb.isEmpty()) {
            break;
        }
        Node* b = pb.dequeue();
        for (int i = 0; i < REFS_IN_NODE; i++) {
            Node* p = b->refs[i];
            if (p != nullptr && !nodeIsVisited(p)) { p->header = VISITED; stack.push(p); }
        }
    }
}