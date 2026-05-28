#include <stdio.h>
#include <stdlib.h>
#include "gen_hash.h"

typedef struct Node{
    void* m_key;
    void* m_value;
    struct Node* m_next;
} Node;

typedef struct HashMap {
   Node** m_buckets;//array of pointers to nodes
   size_t m_capacity;//number of buckets
   size_t m_numsOfItems;//number of key-value pairs in the map
    HashFunction m_hashFunc;//hash function to map keys to bucket indices
    EqualityFunction m_keysEqualFunc;//Compare function to check if two keys are equal
} HashMap;

static int IsPrime(size_t _num) {
    size_t i;
    if (_num <= 1) return 0;
    if (_num % 2 == 0 && _num > 2) return 0;
    
    for (i = 3; i * i <= _num; i += 2) {
        if (_num % i == 0) return 0;
    }
    return 1;
}
static size_t GetNextPrime(size_t _num) {
    while (!IsPrime(_num)) {
        _num++;
    }
    return _num;
}

Node* CreateNode(const void* _key, const void* _value) {
    Node* newNode = (Node*) malloc(sizeof(Node));
    if (newNode == NULL) return NULL;
    newNode->m_key = (void*) _key;
    newNode->m_value = (void*) _value;
    newNode->m_next = NULL;
    return newNode;
}
    
HashMap* HashMap_Create(size_t _size, HashFunction _hashFunc, EqualityFunction _keysEqualFunc){
    //validate input parameters
    if (_size == 0 || _hashFunc == NULL || _keysEqualFunc == NULL) {
        return NULL;
    }

    HashMap* map = (HashMap*) malloc(sizeof(HashMap));
    if (map == NULL) return NULL;

    //allocate buckets array with size of 1.3 times the requested size to reduce collisions
    _size = GetNextPrime(_size*1.3);
    map->m_buckets = (Node**) calloc(_size, sizeof(Node*));
    if (map->m_buckets == NULL) {
        free(map);
        return NULL;
    }

    map->m_capacity = _size;//actual capacity of the map after rounding to nearest prime number
    map->m_numsOfItems = 0;//initially no key-value pairs in the map
    map->m_hashFunc = _hashFunc;//hash function for keys
    map->m_keysEqualFunc = _keysEqualFunc;//equality check function for keys

    return map;
}

void HashMap_Destroy(HashMap** _map, void (*_keyDestroy)(void* _key), void (*_valDestroy)(void* _value)){
      //validate input parameters
    if (_map == NULL || *_map == NULL) return;

   size_t i;
   Node* current, *next;

   //iterate over each bucket and free all nodes
    for(i = 0; i<(*_map)->m_capacity; i++){
        current = (*_map)->m_buckets[i];
        while(current != NULL){
            next = current->m_next;//save pointer to next node before freeing current
            if(_keyDestroy != NULL) _keyDestroy(current->m_key);
            if(_valDestroy != NULL) _valDestroy(current->m_value);
            free(current);
            current = next;
        }
    }

    free((*_map)->m_buckets);//free buckets array
    free(*_map);//free map struct
    *_map = NULL;//set caller's pointer to null to prevent dangling pointer

}

Map_Result HashMap_Insert(HashMap* _map, const void* _key, const void* _value){
    //validate input parameters
    if (_map == NULL) return MAP_UNINITIALIZED_ERROR;
    if (_key == NULL) return MAP_KEY_NULL_ERROR;

    size_t index;
    Node* newNode, *current;

    //check if key already exists in the map to prevent duplicates
    index = _map->m_hashFunc(_key) % _map->m_capacity;//
    current = _map->m_buckets[index];
    while (current != NULL) {
        if(_map->m_keysEqualFunc(current->m_key, _key)) {
            return MAP_KEY_DUPLICATE_ERROR;//key already exists in the map
        }
        current = current->m_next;
}
    newNode = CreateNode(_key, _value);
    if (newNode == NULL) return MAP_ALLOCATION_ERROR;

    newNode->m_next = _map->m_buckets[index];//insert new node at the beginning of the linked list for the bucket
    _map->m_buckets[index] = newNode;//update bucket to point to the
    _map->m_numsOfItems++;//increment number of items in the map

    return MAP_SUCCESS;
}

Map_Result HashMap_Remove(HashMap* _map, const void* _searchKey, void** _pKey, void** _pValue){
    //validate input parameters
    if (_map == NULL) return MAP_UNINITIALIZED_ERROR;
    if (_searchKey == NULL) return MAP_KEY_NULL_ERROR;
    if (_pKey == NULL || _pValue == NULL) return MAP_ALLOCATION_ERROR;

    size_t index = _map->m_hashFunc(_searchKey) % _map->m_capacity;
    Node* current, *prev = NULL;
    current = _map->m_buckets[index];

    //search for the key in the linked list for the bucket
    while( current != NULL){
        if (_map->m_keysEqualFunc(current->m_key, _searchKey)){
            //key found, remove node from linked list
            if (prev == NULL) {
                _map->m_buckets[index] = current->m_next;//removing first node in the bucket
            } else {
                prev->m_next = current->m_next;//bypass current node to remove it from the list
            }
            *_pKey = current->m_key;//return key to caller
            *_pValue = current->m_value;//return value to caller
            free(current);//free the removed node
            _map->m_numsOfItems--;//decrement number of items in the map
            return MAP_SUCCESS;
        }
        prev = current;
        current = current->m_next;
    }
    return MAP_KEY_NOT_FOUND_ERROR;
}

Map_Result HashMap_Find(const HashMap* _map, const void* _key, void** _pValue){
    if (_map == NULL) return MAP_UNINITIALIZED_ERROR;
    if (_key == NULL) return MAP_KEY_NULL_ERROR;
    if (_pValue == NULL) return MAP_ALLOCATION_ERROR;

    size_t index = _map->m_hashFunc(_key) % _map->m_capacity;
    Node* current = _map->m_buckets[index];

    while(current != NULL){
        if(_map->m_keysEqualFunc(current->m_key, _key)){
            *_pValue = current->m_value;//return value associated with the key
            return MAP_SUCCESS;
        }
        current = current->m_next;
    }
    return MAP_KEY_NOT_FOUND_ERROR;
}

size_t HashMap_Size(const HashMap* _map) {
    if (_map == NULL) {
        return 0;
    }
    return _map->m_numsOfItems;
}

size_t HashMap_ForEach(const HashMap* _map, KeyValueActionFunction _action, void* _context) {
    // Input validation 
    if (_map == NULL || _action == NULL) {
        return 0;
    }
    size_t i, count = 0;
    Node* currNode;

    //Outer loop: Iterate over all the buckets in the physical array */
    for (i = 0; i < _map->m_capacity; ++i) {
        currNode = _map->m_buckets[i];
        
        // Inner loop: Iterate over the linked list in the current bucket 
        while (currNode != NULL) {
            count++; /* Count the number of times the action function is invoked */

            /* Invoke the user-provided action function on the key and value.
               Note: The value is passed without const so it can be modified. */
            if (_action((const void*)currNode->m_key, currNode->m_value, _context) == 0) {
                return count; // The user action requested to stop the iteration 
            }
            
            currNode = currNode->m_next;
        }
    }

    return count; // Iterated over the entire map successfully 
}