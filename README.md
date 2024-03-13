# Using prefetch buffer for graph traversal speedup

This small investigation was inspired by
> Chen-Yong Cher, Antony L. Hosking, and T. N. Vijaykumar. 2004. Software prefetching for mark-sweep garbage collection: hardware analysis and software redesign.
> 
Link: [pupupu](https://engineering.purdue.edu/~vijay/papers/2004/gc.pdf)

## Problem

* You need to **traverse a graph** (more specifically, we'll talk about DFS). You use **bit inside graph node** to mark it as visited.
* If your graph has a topology that's rather complex (or close to random) your memory accesses will be unpredictable for hardware prefetch module of CPU.
*  Hypothesis: traversal speed (aka throughput) will be bounded by RAM speed (lots of cache misses)

## Purpose of the work

* Check if hypothesis is actually valid
* Check if software prefetch tactics can help with this problem
* Have fun!

## Simulator

* Written in C/C++ (honestly just C with classes)
* Allocates N Gb of memory for graph nodes
  * Node has header (with bit for marking) and M fields
  * Edges are set randomly
* Simulator does:
  * Builds graph
  * Marks root nodes (X nodes selected randomly)
  * Makes traversal (closure)
  * Counts visited nodes, prints results
 
## Test stand

For demonstrated results I used:

Intel(R) Core(TM) i7-8700 (Coffee Lake) <br>
Virtual threads: 12 <br>
Freq: 3.20 GHz <br>
L1 (i/d): 32K <br>
L2: 256K <br>
L3: 12288K <br>

## Baseline traversal: mark on push

```c++
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
```

Disadvantages:
* Doing memory access into node before push, repeated access occurs after an unlimited period of time. Can cause lots of cache misses.

## Baseline traversal: mark on pop

```c++
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
```

Disadvantages:
* Node will be processed several times if there are several edges pointing to this node.

## Mark on push vs. on pop

* Number of nodes: 10 526 880
* Number of edges: 52 631 749

| Mark on push (ms) | Mark on pop (ms) |
|-------------------|------------------|
| 3260              | 3630             |
| 3265              | 3594             |
| 3257              | 3593             |

* Trivial **mark on pop** sucks! Except that we have to do extra work, we are also limited by memory: before marking and processing fields of node we need to wait for node to be loaded from RAM.
* Can we improve this?

## Prefetch buffer

* Exact implementation from [Cher]
* Core idea: use FIFO on top of mark stack.
  * After poping from stack prefetch node and append it to FIFO.
  * Get next node for processing by removing head of FIFO.

![Screenshot from 2024-03-14 01-33-03](https://github.com/denisenes/excelsior-hackday-2023/assets/55022112/f25facc7-93dc-4ef9-a424-60497a927b3d)

```c++
void enqueueAndPrefetch(Node* node) {
    assert(occupied < PB_SIZE);
    
    buffer[end] = node;
    _m_prefetchw(node);

    advanceEnd();
}

Node* dequeue() {
    assert(!isEmpty());
    Node* node = buffer[start];

    advanceStart();
    return node;
}
```

* Prefetch buffer -- simple circular buffer
* _m_prefetchw is translated to [PREFETCHT0](https://docs.oracle.com/cd/E19120-01/open.solaris/817-5477/epmpw/index.html) instruction
* Buffer size was chosen by gods will (128). Should be investigated more properly. 

## Results

| Mark on push (ms) | Mark on pop (ms) | Mark on pop with Prefetch Buffer |
|-------------------|------------------|----------------------------------|
| 3260              | 3630             | **2560**                         |
| 3265              | 3594             | **2566**                         |
| 3257              | 3593             | **2564**                         |

And actually that works!

* Mark on push with Prefetch Buffer has the same throughput as baseline version. Not interesting.
* Nodes size affects results:
  * For small nodes (2-10 fields) Prefetch Buffer can increase traversal by ~40%
  * For big nodes (1024 fields) can be worse by ~1-2%
  * For big nodes it can be useful to prefetch next cache lines while scanning fields. Should be investigated more properly. 
 
## Conclusion

* Traversing random graph we are really limited by memory
* Prefetch buffer can make it faster
* But it doesn't give you benefits always

To do next:
* Check dependence on the prefetch buffer size
* Check for more interesting graph topologies (heap dumps?)
* Check on different hardware
