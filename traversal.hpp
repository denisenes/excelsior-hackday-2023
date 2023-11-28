#pragma once

#include "main.hpp"
#include "prefetch_buffer.hpp"

typedef struct NodeStack {
    int sp;
    Node* buffer[NODES_N*3];

    public:
        void push(Node* node) {
            assert(this->sp < NODES_N * 2);
            this->buffer[this->sp] = node;
            this->sp++;
        }

        Node* pop() {
            assert(this->sp > 0);
            return this->buffer[--this->sp];
        }

        char isEmpty() {
            return this->sp == 0;
        }
} NodeStack;

typedef struct FieldStack {
    int sp;
    Node** buffer[NODES_N];

    public:
        void push(Node** node) {
            assert(this->sp < TRAVERSAL_STACK_SIZE);
            this->buffer[this->sp] = node;
            this->sp++;
        }

        Node** pop() {
            assert(this->sp > 0);
            return this->buffer[--this->sp];
        }

        char isEmpty() {
            return this->sp == 0;
        }
} FieldStack;

void collectsRootsWithoutMarking(Node* graph, NodeStack& stack);
void collectRootsWithMarking(Node* graph, NodeStack& stack);

class Traversal {
    public:
        virtual void traverseGraph() = 0;
        virtual void collectRoots(Node* graph)  = 0;

};

class BaselineOnPush : public Traversal {
    public:
        static NodeStack stack;

        void traverseGraph();
        void collectRoots(Node* graph) {
            collectRootsWithMarking(graph, stack);
        }
};

class BaselineOnPop : public Traversal {
    static NodeStack stack;

    void traverseGraph();
    void collectRoots(Node* graph) {
        collectsRootsWithoutMarking(graph, stack);
    }
};

class PrefetchBufferOnPop : public Traversal {
    public:
        static NodeStack stack;
        static PrefetchBuffer pb;
        
        void traverseGraph();
        void collectRoots(Node* graph) {
            collectsRootsWithoutMarking(graph, stack);
        }
};

class PrefetchBufferOnPush : public Traversal {
    public:
        static NodeStack stack;
        static PrefetchBuffer pb;
        
        void traverseGraph();
        void collectRoots(Node* graph) {
            collectRootsWithMarking(graph, stack);
        }
};