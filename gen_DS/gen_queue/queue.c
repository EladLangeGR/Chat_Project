#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

typedef struct Queue{
     void** m_items;
    size_t m_size;
    size_t m_head;
    size_t m_tail;
    size_t m_nItems;
}Queue;

Queue* QueueCreate (size_t _size){
    Queue* queue = NULL;
    if(_size == 0) return NULL;
    
    queue = (Queue*)malloc(sizeof(Queue));
    if(queue == NULL) return NULL;
    queue->m_items = (void**)malloc(_size * sizeof(void*));
    if(queue->m_items == NULL){
        free(queue);
        return NULL;
    }
    queue->m_size = _size;
    queue->m_head = 0;
    queue->m_tail = 0;
    queue->m_nItems = 0;
    return queue;   
}

void QueueDestroy(Queue** _queue, DestroyItem _itemDestroy){
    if(_queue == NULL || *_queue == NULL) return;
    if(_itemDestroy != NULL){
        while((*_queue)->m_nItems > 0){
            _itemDestroy((*_queue)->m_items[(*_queue)->m_head]);
            (*_queue)->m_head = ((*_queue)->m_head + 1) % (*_queue)->m_size;
            (*_queue)->m_nItems--;
        }
    }
    free((*_queue)->m_items);
    free(*_queue);
    *_queue = NULL;
}

QueueResult QueueInsert(Queue* _queue, void* _item){
    if(_queue == NULL) return QUEUE_UNINITIALIZED_ERROR;
    if(_item == NULL) return QUEUE_DATA_UNINITIALIZED_ERROR;
    if(_queue->m_nItems == _queue->m_size) return QUEUE_OVERFLOW_ERROR;
    
    _queue->m_items[_queue->m_tail] = _item;
    _queue->m_tail = (_queue->m_tail + 1) % _queue->m_size;
    _queue->m_nItems++;
    return QUEUE_SUCCESS;
}

QueueResult QueueRemove(Queue* _queue, void** _item){
    if(_queue == NULL) return QUEUE_UNINITIALIZED_ERROR;
    if(_item == NULL) return QUEUE_DATA_UNINITIALIZED_ERROR;
    if(_queue->m_nItems == 0) return QUEUE_DATA_NOT_FOUND_ERROR;
    
    *_item = _queue->m_items[_queue->m_head];
    _queue->m_head = (_queue->m_head + 1) % _queue->m_size;
    _queue->m_nItems--;
    return QUEUE_SUCCESS;
}

size_t QueueIsEmpty(Queue* _queue){
    if(_queue == NULL) return 1;
    return (_queue->m_nItems == 0) ? 1 : 0;
}

size_t QueueForEach(Queue* _queue, ActionFunction _action, void* _context){
    size_t count = 0;
    size_t i, index;
    if(_queue == NULL || _action == NULL) return 0;

    for(i = 0; i < _queue->m_nItems; i++){
        index = (_queue->m_head + i) % _queue->m_size;
        count++;
        if(_action(_queue->m_items[index], _context) == 0){
            break;
        }
    }
    return count;
}