#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"
static List arrayOfLists[LIST_MAX_NUM_HEADS];
static Node arrayOfNodes[LIST_MAX_NUM_NODES];
static int countOfLists = 0;
static int countOfNodes = 0;

static int freed_heads = 0;
static int freed_nodes = 0;
static int indexesOf_Freed_Heads[LIST_MAX_NUM_HEADS];
static int indexesOf_Freed_Nodes[LIST_MAX_NUM_NODES];
// freed heads and nodes are only available if they have been deleted at some point


List* List_create() {
    if (countOfLists >= LIST_MAX_NUM_HEADS) {
        return NULL;
    }
    List* ret;
    if (freed_heads > 0) {
        freed_heads--;
        ret = &arrayOfLists[indexesOf_Freed_Heads[freed_heads]]; // find the free head through the list of indexes
        ret->listIndex = indexesOf_Freed_Heads[freed_heads];
        indexesOf_Freed_Heads[freed_heads] = -1;
    }
    else {
        ret = &arrayOfLists[countOfLists];
        ret->listIndex = countOfLists;
    }

    ret->head = NULL;
    ret->current = NULL;
    ret->nodeCount = 0;
    ret->inBounds = LIST_OOB_START;
    countOfLists++;


    if (ret == NULL)
        return NULL;
    return ret;
}

// Returns the number of items in pList.
int List_count(List* pList) {
    return pList->nodeCount;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
//Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
    if (pList->head == NULL) {
        pList->current = NULL;
        return NULL;
    }
    pList->inBounds = LIST_VALID;
    pList->current = pList->head;
    return pList->current->item;
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
    if (pList->head == NULL) {
        pList->current = NULL;
        return NULL;
    }
    pList->inBounds = LIST_VALID;
    pList->current = pList->tail;
    return pList->current->item;
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList) {
    // IMPLEMENT: if beyond return null
    if (pList->head == NULL)
        return NULL;
    if (pList->current == NULL) {
        if (pList->inBounds == LIST_OOB_START) {
            pList->inBounds = LIST_VALID;
            pList->current = pList->head;
            return pList->current->item;
        }
        if (pList->inBounds == LIST_OOB_END) {
            return NULL;
        }
    }
    else if (pList->current->next != NULL)
    {
        pList->current = pList->current->next;
        return pList->current->item;
    }
    // else: if next goes beyond
    pList->inBounds = LIST_OOB_END;
    pList->current = NULL;
    return NULL;
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList) {
    if (pList->head == NULL)
        return NULL;
    if (pList->current == NULL)
    {
        if (pList->inBounds == LIST_OOB_END) {
            pList->current = pList->tail;
            pList->inBounds = LIST_VALID;
            return pList->current->item;
        }
        if (pList->inBounds == LIST_OOB_START)
            return NULL;
    }
    else if (pList->current->prev != NULL) {
        pList->current = pList->current->prev;
        return pList->current->item;
    }
    // else: if prev goes OOB to before the start
    pList->current = NULL;
    pList->inBounds = LIST_OOB_START;
    return NULL;
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList) {
    if (pList->current == NULL)
    {
        return NULL;
    }
    return pList->current->item;
}
// helper functions:
static Node* createNode(void* pItem, List* pList) {
    Node* newNode;
    if (freed_nodes > 0) {
        newNode = &arrayOfNodes[indexesOf_Freed_Nodes[freed_nodes - 1]];
        freed_nodes--;
    }
    else
        newNode = &arrayOfNodes[countOfNodes];
    newNode->item = pItem;
    newNode->nodeIndex = countOfNodes;
    newNode->parentList = pList->listIndex;
    // unlink, this is for when reusing freed nodes
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}
static void add_on_empty(List* pList, Node* aNode) {
    pList->head = aNode;
    pList->tail = aNode;
    pList->inBounds = LIST_VALID;
}
static void add_to_start(List* pList, Node* aNode) {
    pList->head->prev = aNode;
    aNode->next = pList->head;
    pList->head = aNode;
    pList->inBounds = LIST_VALID;
}
static void add_to_end(List* pList, Node* aNode) {
    pList->tail->next = aNode;
    aNode->prev = pList->tail;
    pList->tail = aNode;
    pList->inBounds = LIST_VALID;
}
// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_add(List* pList, void* pItem) {
    //if current is before start:
    // add item to the start of the underlying array
    if (countOfNodes == LIST_MAX_NUM_NODES) {
        return -1;
    }
    // "create" newNode by taking one from the arrayOfNodes
    // set item and index and record parent
    Node* newNode = createNode(pItem, pList);
    if (newNode == NULL)
        return -1;

    // if newNode is the first added node
    if (pList->nodeCount == 0) {
        add_on_empty(pList, newNode);
    }

    // if current item is before the start of the list or is the head
    // add newNode to the start and make it the new head
    else if (pList->inBounds == LIST_OOB_START) {
        add_to_start(pList, newNode);
    }

    // if current item is beyond the end of the list or is the tail
    // add newNode to the end of the list and make it the tail
    else if (pList->inBounds == LIST_OOB_END || pList->current == pList->tail) {
        add_to_end(pList, newNode);
    }

    // if current item is within the list
    // add it after the current item and make it the current item
    else {
        Node* oldNode = pList->current;
        newNode->next = oldNode->next;
        oldNode->next = newNode;
        newNode->prev = oldNode;
        // if oldNode was the head
        if (oldNode == pList->head) {
            pList->head->next = newNode;
        }
        // if oldNode was not the tail, link the next item's prev to the newNode (doubly linked list)
        if (newNode->next != NULL) {
            newNode->next->prev = newNode;
        }
    }
    // increment total count of pool of nodes, increment count within this list
    // make current pointer point to newNode
    countOfNodes++;
    pList->nodeCount++;
    pList->current = newNode;
    return 0;

}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem) {
    if (countOfNodes == LIST_MAX_NUM_NODES) {
        return -1;
    }
    Node* newNode = createNode(pItem, pList);
    if (newNode == NULL)
        return -1;

    if (pList->nodeCount == 0) {
        add_on_empty(pList, newNode);
    }

    else if (pList->inBounds == LIST_OOB_START || pList->current == pList->head) {
        add_to_start(pList, newNode);
    }

    else if (pList->inBounds == LIST_OOB_END) {
        add_to_end(pList, newNode);
    }

    else {
        Node* oldNode = pList->current;
        newNode->prev = oldNode->prev;
        oldNode->prev = newNode;
        newNode->next = oldNode;
        if (newNode->prev != NULL)
            newNode->prev->next = newNode;
    }
    countOfNodes++;
    pList->nodeCount++;
    pList->current = newNode;

    return 0;
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
    if (countOfNodes == LIST_MAX_NUM_NODES) {
        return -1;
    }
    Node* newNode = createNode(pItem, pList);
    if (newNode == NULL)
        return -1;

    if (pList->head == NULL) {
        add_on_empty(pList, newNode);
    }
    else {
        add_to_end(pList, newNode);
    }
    pList->current = newNode;
    pList->nodeCount++;
    countOfNodes++;
    return 0;
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
    if (countOfNodes == LIST_MAX_NUM_NODES) {
        return -1;
    }
    Node* newNode = createNode(pItem, pList);
    if (newNode == NULL)
        return -1;

    if (pList->head == NULL) {
        add_on_empty(pList, newNode);
    }
    else {
        add_to_start(pList, newNode);
    }
    pList->current = newNode;
    pList->nodeCount++;
    countOfNodes++;
    return 0;
}

//Return current item and take it out of pList. Make the next item the current one.
//If the current pointer is before the start of the pList, or beyond the end of the pList,
//then do not change the pList and return NULL.
void* List_remove(List* pList) {
    if (pList->inBounds != LIST_VALID)
        return NULL;

    Node* toRemove = pList->current;
    // save item in removed node
    void* ret = toRemove->item;

    // decrement nodeCount in list, check if list is now empty
    pList->nodeCount--;
    if (pList->nodeCount == 0) {
        pList->inBounds = LIST_OOB_START;
        pList->current = NULL;
        pList->head = NULL;
        pList->tail = NULL;
    }
    else { //if pList is not empty yet 
        // if the removed node was at the tail, next item will set it beyond the list
        if (toRemove == pList->tail) {
            pList->tail = toRemove->prev;
            pList->current = NULL;
            pList->inBounds = LIST_OOB_END;
        }
        else {
            // make next item the current one
            pList->current = toRemove->next;
            // link the previous and next items
            toRemove->next->prev = toRemove->prev;
            // if toRemove was head
            if (toRemove == pList->head)
                pList->head = pList->current;
            else
                toRemove->prev->next = toRemove->next;

        }
    }
    indexesOf_Freed_Nodes[freed_nodes] = toRemove->nodeIndex;
    freed_nodes++;
    // decrement, making final position free to use
    countOfNodes--;
    return ret;
}
// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2) {
    // if pList1 is empty, but pList2 is not, assign pList 2 to it
    int toRemove = pList2->listIndex;
    if (pList1->head == NULL && pList2->head != NULL) {
        pList1->nodeCount = pList2->nodeCount;
        pList1->head = pList2->head;
        pList1->current = pList2->current;
        pList1->tail = pList2->tail;
    }
    else if (pList1->head != NULL) {
        pList1->tail->next = pList2->head;
        pList1->nodeCount = pList1->nodeCount + pList2->nodeCount;
        if (pList2->nodeCount != 0) {
            pList2->head->prev = pList1->tail;
            pList1->tail = pList2->tail;
        }
    }
    indexesOf_Freed_Heads[freed_heads] = toRemove;
    freed_heads++;
    //make plist2 head available
    pList2->head = NULL;
    pList2->nodeCount = 0;
    pList2 = NULL;
    countOfLists--;
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn) {
    assert(pList != NULL);
    pList->current = pList->head;
    while (pList->current != NULL) {
        int toRemove = pList->current->nodeIndex;
        (*pItemFreeFn)(pList->current->item);
        pList->current->prev = NULL;
        pList->current = pList->current->next;
        // free up the node for future use
        arrayOfNodes[toRemove].next = NULL;
        arrayOfNodes[toRemove].prev = NULL;
        freed_nodes++;
        indexesOf_Freed_Nodes[freed_nodes - 1] = toRemove;
        countOfNodes--;
    }
    // free up the head
    indexesOf_Freed_Heads[freed_heads] = pList->listIndex;
    freed_heads++;
    pList->head = NULL;
    pList->tail = NULL;
    pList = NULL;
    // decrement lists to make thhe next one available for use;
    countOfLists--;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList) {
    if (pList->head == NULL)
        return NULL;
    Node* toRemove = pList->tail;
    void* ret = toRemove->item;
    // if there is only one head/tail
    if (pList->nodeCount == 1) {
        pList->head = NULL;
        pList->tail = NULL;
    }

    // if there is more than one item
    else {
        toRemove->prev->next = toRemove->next;
        pList->tail = toRemove->prev;
    }
    pList->current = pList->tail;
    freed_nodes++;
    indexesOf_Freed_Nodes[freed_nodes - 1] = toRemove->nodeIndex;
    countOfNodes--;
    pList->nodeCount--;
    return ret;
}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 

// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    if (pList->inBounds == LIST_OOB_END) {
        return NULL;
    }
    if (pList->inBounds == LIST_OOB_START) {
        List_first(pList);
    }
    while (pList->current != NULL) {
        if ((*pComparator)(pList->current->item, pComparisonArg) == 1) {
            return pList->current->item;
        }
        List_next(pList);
    }
    return NULL;
}