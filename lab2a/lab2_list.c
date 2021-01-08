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

#include "SortedList.h"



long num_threads=1;
long num_iterations=1;
int opt_yield=0;
int spin_lock=0;
char sync_opion;
pthread_mutex_t mutex;
SortedList_t* list;
SortedListElement_t* elements;
 
void signal_handler(int sig) {
    //just to remove unused warning
    if(sig>0){
        sig=SIGSEGV;
    }
    fprintf(stderr,"Error: segmentation fault detected\n");
    exit(2);

}




void* routine(void* p){
    int head = *((int*)p);
    int length=0;
        switch(sync_opion){
            case 'm':
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    pthread_mutex_lock(&mutex);
                    SortedList_insert(list, &elements[i]);
                    pthread_mutex_unlock(&mutex);
                    
                }
                
                
                pthread_mutex_lock(&mutex);
                length = SortedList_length(list);
                pthread_mutex_unlock(&mutex);
                
                if(length < 0){
                        fprintf(stderr, "Error: corrupted list %s\n", strerror(errno));
                        exit(2);
                }
                
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    pthread_mutex_lock(&mutex);
                    SortedListElement_t* look_up_element = SortedList_lookup(list, elements[i].key);
                    
                    if(look_up_element == NULL){
                        fprintf(stderr, "Error: failed to look up the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                
                    if(SortedList_delete(look_up_element) != 0){
                        fprintf(stderr, "Error:failed to deleted the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                    pthread_mutex_unlock(&mutex);
                }
                
                break;
            case 's':
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    while (__sync_lock_test_and_set(&spin_lock, 1));
                    SortedList_insert(list, &elements[i]);
                    __sync_lock_release(&spin_lock);
                    
                }
               
                
                while (__sync_lock_test_and_set(&spin_lock, 1));
                length = SortedList_length(list);
                __sync_lock_release(&spin_lock);
                
                if(length < 0){
                        fprintf(stderr, "Error: corrupted list %s\n", strerror(errno));
                        exit(2);
                }
                
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    while (__sync_lock_test_and_set(&spin_lock, 1));
                    SortedListElement_t* look_up_element = SortedList_lookup(list, elements[i].key);
                    
                    if(look_up_element == NULL){
                        fprintf(stderr, "Error: failed to look up the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                
                    if(SortedList_delete(look_up_element) != 0){
                        fprintf(stderr, "Error:failed to deleted the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                    __sync_lock_release(&spin_lock);
                }
                break;
            default:
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    SortedList_insert(list, &elements[i]);
                }
                
                length = SortedList_length(list);
                if(length < 0){
                        fprintf(stderr, "Error: corrupted list %s\n", strerror(errno));
                        exit(2);
                }
                
                for(long i = head*num_iterations; i < (head+1)*num_iterations; i++){
                    
                    SortedListElement_t* look_up_element = SortedList_lookup(list, elements[i].key);
                    
                    if(look_up_element == NULL){
                        fprintf(stderr, "Error: failed to look up the specified element: %s\n", strerror(errno));
                        exit(2);
                    }
                
                    if(SortedList_delete(look_up_element) != 0){
                        fprintf(stderr, "Error:failed to deleted the specified element: %s\n", strerror(errno));
                        exit(2);
                    }

                }
                
        }
    
    return NULL;
}




int main(int argc, char* argv[]) {
    struct option opt[] = {
            {"threads", required_argument, NULL, 't'},
            {"iterations", required_argument, NULL, 'i'},
            {"yield", required_argument, NULL, 'y'},
            {"sync", required_argument, NULL, 's'},
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
            default:
                fprintf(stderr, "Error: Unreognized argument detected %s\n", strerror(errno));
                exit(1);
        }
        c = getopt_long(argc, argv, "", opt, &option_index);
    }
    
    long num_elements = num_threads * num_iterations;
    
    list = (SortedList_t*) malloc(sizeof(SortedList_t));
       if(list==NULL){
           fprintf(stderr, "Error: failed to initialize the list %s\n", strerror(errno));
       }
    list->key = NULL;
    list->prev = list;
    list->next = list;
    
    elements = malloc(sizeof(SortedListElement_t) * num_elements);
    if (elements == NULL) {
        fprintf(stderr, "Error: failed to allocate enough memory for the list elements %s\n", strerror(errno));
            exit(1);
    }
    
    srand((unsigned int) time(NULL));
    char a[] = "abcdefghijklmnopqrstuvwxyz";
    char element_key[2];
    for(long i =0;i<num_elements;i++){
        element_key[0]= a[rand()%26];
        element_key[1] = '\0';
        elements[i].key = element_key;
    }
    
    if (sync_opion == 'm') {
            if (pthread_mutex_init(&mutex, NULL) != 0){
                fprintf(stderr, "Error: failed to create the mutex %s\n", strerror(errno));
                exit(1);
            }
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
    
    for (long i = 0; i < num_threads; i++){
           if(pthread_join(thread_ids[i], NULL) != 0){
               fprintf(stderr, "Error: failed to join the threads %s\n", strerror(errno));
               exit(1);
           }
    }
    
    if(clock_gettime(CLOCK_MONOTONIC, &end)<0){
        fprintf(stderr, "Error: failed to get the ending time %s\n", strerror(errno));
        exit(1);
    }
    
    if(SortedList_length(list) != 0){
            fprintf(stderr, "Error: failed to clean up the list, the list may be corrupted %s\n", strerror(errno));
            exit(2);
    }
    
    int num_list=1;
    long num_operations = num_threads * num_iterations * 3;
    long long diff_sec = end.tv_sec - begin.tv_sec;
    long long diff_nsec = end.tv_nsec - begin.tv_nsec;
    long long run_time = 1000000000L * diff_sec + diff_nsec;
    long long average_time = run_time/num_operations;
    
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
    
    fprintf(stdout, ",%ld,%ld,%d,%ld,%lld,%lld\n", num_threads, num_iterations, num_list, num_operations, run_time, average_time);
    
    
    if (sync_opion == 'm')
        pthread_mutex_destroy(&mutex);
    
    free(elements);
    free(thread_ids);
    free(list);
    
    exit(0);
}
