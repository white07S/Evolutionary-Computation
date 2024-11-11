#ifndef DELTA_LOCAL_SEARCH_H
#define DELTA_LOCAL_SEARCH_H

#include "utils.h"
#include "algorithms.h"

// Define DeltaLocalSearch struct
typedef struct
{
    Algo base;
    int method_index; // For identification purposes
} DeltaLocalSearch;

// Function to create a DeltaLocalSearch algorithm
DeltaLocalSearch* create_DeltaLocalSearch(int method_index);




typedef struct
{
    int i;
    int j;
    int type; // 0 for intra-route (2-opt), 1 for inter-route
    int delta;
    int edge_u1, edge_u2; // Edge u1-u2 to be removed
    int edge_v1, edge_v2; // Edge v1-v2 to be removed
    int reversed; // 1 if edges are reversed, 0 otherwise
} Move;


// Define the structure for a doubly linked list node
typedef struct Node {
    Move move;
    struct Node* next;
    struct Node* prev;
} Node;

// Define the structure for the doubly linked list
typedef struct {
    Node* head;    // Pointer to the first (smallest) element
} SortedDoublyLinkedList;

// Function declarations
void initList(SortedDoublyLinkedList* list);
void insert(SortedDoublyLinkedList* list, Move move);
Move* getSmallest(SortedDoublyLinkedList* list);
void deleteSmallest(SortedDoublyLinkedList* list);
void deleteNode(SortedDoublyLinkedList* list, Node* node);
void printList(SortedDoublyLinkedList* list);


#endif // DELTA_LOCAL_SEARCH_H
