//NAME: JIAXUAN XUE
//EMAIL: nimbusxue1015@g.ucla.edu
//ID: 705142227

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include "SortedList.h"



long num_threads=1;
long num_iterations=1;
long num_lists = 1;
int* spin_lock;
char sync_opion;
pthread_mutex_t* mutex;
SortedList_t* lists;
SortedListElement_t* elements;
 
void signal_handler(int sig) {
    //just to remove unused warning
    if(sig>0){
        sig=SIGSEGV;
    }
    fprintf(stderr,"Error: segmentation fault detected\n");
    exit(2);

}

long hash(const char *key){
    int key_value = (int)*key;
    return (key_value*100) % num_lists;
}



void* routine(void* p){
    int head = *((int*)p);
    int length=0;
    long wait_time = 0;
    struct timespec begin, end;
        switch(sync_opion){
            case 'm':
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    long hash_value=hash(elements[i].key);
                    if(clock_gettime(CLOCK_MONOTONIC, &begin)<0){
                        fprintf(stderr, "Error: failed to get the starting time %s\n", strerror(errno));
                        exit(1);
                    }
                    pthread_mutex_lock(mutex+hash_value);
                    if(clock_gettime(CLOCK_MONOTONIC, &end)<0){
                        fprintf(stderr, "Error: failed to get the ending time %s\n", strerror(errno));
                        exit(1);
                    }
                    SortedList_insert(lists+hash_value, &elements[i]);
                    pthread_mutex_unlock(mutex+hash_value);
                    wait_time += 1000000000 * (end.tv_sec - begin.tv_sec) + end.tv_nsec - begin.tv_nsec;
                    
                }
                
                for(long i=0;i<num_lists;i++){
                    if(clock_gettime(CLOCK_MONOTONIC, &begin)<0){
                        fprintf(stderr, "Error: failed to get the starting time %s\n", strerror(errno));
                        exit(1);
                    }
                    pthread_mutex_lock(mutex+i);
                    if(clock_gettime(CLOCK_MONOTONIC, &end)<0){
                        fprintf(stderr, "Error: failed to get the ending time %s\n", strerror(errno));
                        exit(1);
                    }
                    length += SortedList_length(lists+i);
                    pthread_mutex_unlock(mutex+i);
                    wait_time += 1000000000 * (end.tv_sec - begin.tv_sec) + end.tv_nsec - begin.tv_nsec;

                    if(SortedList_length(lists+i) < 0){
                            fprintf(stderr, "Error: corrupted list %s\n", strerror(errno));
                            exit(2);
                    }
                }

                
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    long hash_value=hash(elements[i].key);
                    if(clock_gettime(CLOCK_MONOTONIC, &begin)<0){
                        fprintf(stderr, "Error: failed to get the starting time %s\n", strerror(errno));
                        exit(1);
                    }
                    pthread_mutex_lock(mutex+hash_value);
                    if(clock_gettime(CLOCK_MONOTONIC, &end)<0){
                        fprintf(stderr, "Error: failed to get the ending time %s\n", strerror(errno));
                        exit(1);
                    }
                    
                    
                    SortedListElement_t* look_up_element = SortedList_lookup(lists+hash_value, elements[i].key);
                    
                    if(look_up_element == NULL){
                        fprintf(stderr, "Error: failed to look up the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                
                    if(SortedList_delete(look_up_element) != 0){
                        fprintf(stderr, "Error:failed to deleted the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                    pthread_mutex_unlock(mutex + hash_value);
                    wait_time += 1000000000 * (end.tv_sec - begin.tv_sec) + end.tv_nsec - begin.tv_nsec;
                }
                
                break;
            case 's':
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    long hash_value=hash(elements[i].key);
                    if(clock_gettime(CLOCK_MONOTONIC, &begin)<0){
                        fprintf(stderr, "Error: failed to get the starting time %s\n", strerror(errno));
                        exit(1);
                    }
                    while (__sync_lock_test_and_set(spin_lock + hash_value, 1));
                    if(clock_gettime(CLOCK_MONOTONIC, &end)<0){
                        fprintf(stderr, "Error: failed to get the ending time %s\n", strerror(errno));
                        exit(1);
                    }
                    SortedList_insert(lists+hash_value, &elements[i]);
                    __sync_lock_release(spin_lock + hash_value);
                    wait_time += 1000000000 * (end.tv_sec - begin.tv_sec) + end.tv_nsec - begin.tv_nsec;
                    
                }
                
                for(long i=0;i<num_lists;i++){
                    if(clock_gettime(CLOCK_MONOTONIC, &begin)<0){
                        fprintf(stderr, "Error: failed to get the starting time %s\n", strerror(errno));
                        exit(1);
                    }
                    while (__sync_lock_test_and_set(spin_lock + i, 1));
                    if(clock_gettime(CLOCK_MONOTONIC, &end)<0){
                        fprintf(stderr, "Error: failed to get the ending time %s\n", strerror(errno));
                        exit(1);
                    }
                    length += SortedList_length(lists+i);
                    __sync_lock_release(spin_lock + i);
                    wait_time += 1000000000 * (end.tv_sec - begin.tv_sec) + end.tv_nsec - begin.tv_nsec;

                    if(SortedList_length(lists+i) < 0){
                            fprintf(stderr, "Error: corrupted list %s\n", strerror(errno));
                            exit(2);
                    }
                }
                
                
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    long hash_value=hash(elements[i].key);
                    if(clock_gettime(CLOCK_MONOTONIC, &begin)<0){
                        fprintf(stderr, "Error: failed to get the starting time %s\n", strerror(errno));
                        exit(1);
                    }
                    while (__sync_lock_test_and_set(spin_lock + hash_value, 1));
                    if(clock_gettime(CLOCK_MONOTONIC, &end)<0){
                        fprintf(stderr, "Error: failed to get the ending time %s\n", strerror(errno));
                        exit(1);
                    }
                    
                    
                    SortedListElement_t* look_up_element = SortedList_lookup(lists+hash_value, elements[i].key);
                    
                    if(look_up_element == NULL){
                        fprintf(stderr, "Error: failed to look up the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                
                    if(SortedList_delete(look_up_element) != 0){
                        fprintf(stderr, "Error:failed to deleted the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                    __sync_lock_release(spin_lock + hash_value);
                    wait_time += 1000000000 * (end.tv_sec - begin.tv_sec) + end.tv_nsec - begin.tv_nsec;
                }
                
                break;
            default:
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    long hash_value=hash(elements[i].key);
                    SortedList_insert(lists+hash_value, &elements[i]);
                }
                
                for(long i=0;i<num_lists;i++){
                    length += SortedList_length(lists+i);
                    if(SortedList_length(lists+i) < 0){
                            fprintf(stderr, "Error: corrupted list %s\n", strerror(errno));
                            exit(2);
                    }
                }

                
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    long hash_value=hash(elements[i].key);
                    SortedListElement_t* look_up_element = SortedList_lookup(lists+hash_value, elements[i].key);
                    
                    if(look_up_element == NULL){
                        fprintf(stderr, "Error: failed to look up the specified element: %s\n", strerror(errno));
                        exit(2);
                    }

                    if(SortedList_delete(look_up_element) != 0){
                        fprintf(stderr, "Error:failed to deleted the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                }
                
                break;
                
        }
    
    return (void*)wait_time;
}




int main(int argc, char* argv[]) {
    struct option opt[] = {
            {"threads", required_argument, NULL, 't'},
            {"iterations", required_argument, NULL, 'i'},
            {"yield", required_argument, NULL, 'y'},
            {"sync", required_argument, NULL, 's'},
            {"lists", required_argument, NULL, 'l'},
            {0, 0, 0, 0}
    };
    
    signal(SIGSEGV, signal_handler);
    int option_index = 0;
    int c = getopt_long(argc, argv, "", opt, &option_index);
    while(1){
        if(c == -1) break;
        switch(c){
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'i':
                num_iterations = atoi(optarg);
                break;
            case 'y':
                for(size_t i  = 0; i < strlen(optarg); i++){
                    switch(optarg[i]){
                        case 'i':
                            opt_yield |= INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield |= DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield |= LOOKUP_YIELD;
                            break;
                        default:
                            fprintf(stderr, "Error: Unreognized yield argument detected %s\n", strerror(errno));
                            exit(1);
                    }
                }
                break;
            case 's':
                sync_opion = optarg[0];
                break;
            case 'l':
                num_lists = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Error: Unreognized argument detected %s\n", strerror(errno));
                exit(1);
        }
        c = getopt_long(argc, argv, "", opt, &option_index);
    }
   
    
    lists = malloc(sizeof(SortedList_t) * num_lists);

    if (lists == NULL) {
        fprintf(stderr, "Error: failed to initialize the list %s\n", strerror(errno));
            exit(1);
    }
    
    
    for (long i = 0; i < num_lists; i++){
           lists[i].prev = &lists[i];
           lists[i].next = &lists[i];
           lists[i].key = NULL;
    }
    
    switch(sync_opion){
        case 'm':
            mutex = malloc(sizeof(pthread_mutex_t) * num_lists);
            if(mutex == NULL){
               fprintf(stderr, "Error: failed to allocate enough space for mutex %s\n", strerror(errno));
                 exit(1);
            }
            for (long i = 0; i < num_lists; i++){
                if(pthread_mutex_init(&mutex[i], NULL) < 0){
                    fprintf(stderr, "Error: failed to create the mutex %s\n", strerror(errno));
                    exit(1);
                }
            }
            break;
        case 's':
            spin_lock = malloc(sizeof(int) * num_lists);
            if(spin_lock == NULL){
                fprintf(stderr, "Error: malloc failed to allocate enough space for spinlock: %s\n", strerror(errno));
                  exit(1);
            }
            for (long i = 0; i < num_lists; i++){
                spin_lock[i]=0;
            }
            break;
        default:
            break;
    }
    
    long num_elements = num_threads * num_iterations;
    
    elements = malloc(sizeof(SortedListElement_t) * num_elements);
    if (elements == NULL) {
        fprintf(stderr, "Error: failed to allocate enough memory for the list elements %s\n", strerror(errno));
            exit(1);
    }
    
    srand((unsigned int) time(NULL));
    char a[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char element_key[2];
    for(long i =0;i<num_elements;i++){
        element_key[0]= a[rand()%52];
        element_key[1] = '\0';
        elements[i].key = element_key;
    }
    
    
    pthread_t* thread_ids = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    
    if (thread_ids == NULL) {
        fprintf(stderr, "Error: failed to allocate enough memory for all the threads %s\n", strerror(errno));
        exit(1);
    }
    
    struct timespec begin, end;
    if(clock_gettime(CLOCK_MONOTONIC, &begin)<0){
        fprintf(stderr, "Error: failed to get the starting time %s\n", strerror(errno));
        exit(1);
    }
    
    int start[num_threads];
    
    for(long i=0;i<num_threads;i++){
        start[i]=i;
    }
    
    
    for (long i = 0; i < num_threads; i++) {
            if (pthread_create(&thread_ids[i], NULL, &routine, &start[i]) != 0) {
                fprintf(stderr, "Error: failed to create the threads %s\n", strerror(errno));
                exit(1);
            }
    }
    
    long wait_times = 0;
    void** wait_time = (void *) malloc(sizeof(void**));
    for (long i = 0; i < num_threads; i++){
           if(pthread_join(thread_ids[i], wait_time) != 0){
               fprintf(stderr, "Error: failed to join the threads %s\n", strerror(errno));
               exit(1);
           }
        wait_times +=(long) *wait_time;
    }
    
    if(clock_gettime(CLOCK_MONOTONIC, &end)<0){
        fprintf(stderr, "Error: failed to get the ending time %s\n", strerror(errno));
        exit(1);
    }
    
    for(long i=0;i<num_lists;i++){
        int len= SortedList_length(lists + i);
        if(len != 0){
            fprintf(stderr, "Error: failed to clean up the list, the list may be corrupted %s\n", strerror(errno));
            exit(2);
        }
    }
    
        
    long num_operations = num_threads * num_iterations * 3;
    long long diff_sec = end.tv_sec - begin.tv_sec;
    long long diff_nsec = end.tv_nsec - begin.tv_nsec;
    long long run_time = 1000000000L * diff_sec + diff_nsec;
    long long average_time = run_time/num_operations;
    long average_wait_time = wait_times/num_operations;
    
    fprintf(stdout, "list");
    
    switch(opt_yield) {
                case 0:
                fprintf(stdout, "-none");
                break;
                case 1:
                fprintf(stdout, "-i");
                break;
               case 2:
                fprintf(stdout, "-d");
                break;
                case 3:
                fprintf(stdout, "-id");
                break;
                case 4:
                fprintf(stdout, "-l");
                break;
                case 5:
                fprintf(stdout, "-il");
                break;
                case 6:
                fprintf(stdout, "-dl");
                break;
                case 7:
                fprintf(stdout, "-idl");
                break;
                default:
                    break;
    }
    
    switch(sync_opion) {
                case 0:
                    fprintf(stdout, "-none");
                    break;
                case 's':
                    fprintf(stdout, "-s");
                    break;
                case 'm':
                    fprintf(stdout, "-m");
                    break;
                default:
                    break;
    }
    
    fprintf(stdout, ",%ld,%ld,%ld,%ld,%lld,%lld,%ld\n", num_threads, num_iterations, num_lists, num_operations, run_time, average_time,average_wait_time);
    
    switch(sync_opion) {
                case 's':
                    free (spin_lock);
                    break;
                case 'm':
                    for (long i = 0; i < num_lists; i++){
                        pthread_mutex_destroy(mutex + i);
                    }
                    free(mutex);
                    break;
                default:
                    break;
    }
    
        
    free(wait_time);
    free(elements);
    free(thread_ids);
    free(lists);
    
    exit(0);
}
