//NAME: JIAXUAN XUE
//EMAIL: nimbusxue1015@g.ucla.edu
//ID: 705142227

#include "SortedList.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sched.h>


void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    //return if list is null
    if (list == NULL || element == NULL)
        return;

    SortedListElement_t *first = list->next;
    
    while (first != list && strcmp(element->key, first->key) >= 0)
        first = first->next;
    
    //critical section
    if (opt_yield & INSERT_YIELD)
        sched_yield();
    
    element->next = first;
    element->prev = first->prev;
    first->prev->next = element;
    first->prev = element;
}

int SortedList_delete( SortedListElement_t *element){
    if(element == NULL || element->next->prev != element || element->prev->next != element)
        return 1;
    
    //critical section
    if (opt_yield & DELETE_YIELD)
        sched_yield();
    
    element->prev->next = element->next;
    element->next->prev = element->prev;

    return 0;
}


SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    if (list == NULL)
        return NULL;
    
    SortedListElement_t* first = list->next;
    while(first != list){
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        
        if (strcmp(key, first->key) == 0){
            return first;
        }else{
            first = first->next;
        }
    }
    return NULL;
}

int SortedList_length(SortedList_t *list) {
    //check for corruption
        if (list == NULL)
            return -1;

        int len = 0;
        SortedListElement_t* first = list->next;

        while (first != list) {
            
            if (first->next->prev != first ||first->prev->next != first)
                return -1;
                   
            if (opt_yield & LOOKUP_YIELD)
                  sched_yield();
            
            len++;
            first = first->next;
        }
        return len;
}
