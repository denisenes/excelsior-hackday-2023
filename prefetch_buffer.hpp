#include <x86intrin.h>
#include <cassert>

#include "main.hpp"

#include <stdio.h>

#define PB_SIZE 128

class PrefetchBuffer {
    public:
        PrefetchBuffer() {
            start = end = occupied = 0;
        }

        bool isFull() {
            assert(occupied <= PB_SIZE);
            return occupied == PB_SIZE;
        }

        bool isEmpty() {
            assert(occupied >= 0);
            return occupied == 0;
        }

        void enqueueAndPrefetch(Node* node) {
            // printf("%d\n", occupied);
            assert(occupied < PB_SIZE);
            
            buffer[end] = node;
            _m_prefetchw(node);

            advanceEnd();
        }

        Node* dequeue() {
            // printf("%d\n", occupied);
            assert(!isEmpty());
            Node* node = buffer[start];

            advanceStart();
            return node;
        }

    private:
        int start;
        int end;
        int occupied;
        Node* buffer[PB_SIZE];

        void advanceEnd() {
            occupied++;
            end++;
            if (end == PB_SIZE)
                end = 0;
        }

        void advanceStart() {
            occupied--;
            start++;
            if (start == PB_SIZE)
                start = 0;
        }
};