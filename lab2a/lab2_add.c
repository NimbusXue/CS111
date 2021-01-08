//NAME: JIAXUAN XUE
//EMAIL: nimbusxue1015@g.ucla.edu
//ID: 705142227

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sched.h>
#include <pthread.h>


long long counter;
long num_threads=1;
long num_iterations=1;
int opt_yield=0;
int spin_lock=0;
pthread_mutex_t mutex;
char sync_opion;

 

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield) {
            sched_yield();
    }
       *pointer = sum;
}


void add_m(long long *pointer, long long value){
    pthread_mutex_lock(&mutex);
    add(pointer, value);
    pthread_mutex_unlock(&mutex);
}

void add_s(long long *pointer, long long value) {
    while (__sync_lock_test_and_set(&spin_lock, 1));
    add(pointer, value);
    __sync_lock_release(&spin_lock);
}


void add_c(long long *pointer, long long value){
    long long old,new;
    do{
        old = counter;
        new = old +value;
        if(opt_yield){
            sched_yield();
        }
    }while(__sync_val_compare_and_swap(pointer, old, new) != old);
    
}

void* routine(void* p){
    for(long i = 0; i < num_iterations; i++){
        switch(sync_opion){
            case 'm':
                add_m(&counter, 1);
                add_m(&counter, -1);
                break;
            case 's':
                add_s(&counter, 1);
                add_s(&counter, -1);
                break;
            case 'c':
                add_c(&counter, 1);
                add_c(&counter, -1);
                break;
            default:
                add(&counter, 1);
                add(&counter, -1);
        }
    }
    return p;
}

void print_output(char* test_name, long num_threads, long num_iterations, long num_operations, long long run_time, long long average_time, long long result){
    fprintf(stdout, "%s,%ld,%ld,%ld,%lld,%lld,%lld\n", test_name, num_threads, num_iterations, num_operations, run_time, average_time, result);
    
}


int main(int argc, char* argv[]) {
    struct option opt[] = {
            {"threads", required_argument, NULL, 't'},
            {"iterations", required_argument, NULL, 'i'},
            {"yield", no_argument, NULL, 'y'},
            {"sync", required_argument, NULL, 's'},
            {0, 0, 0, 0}
    };
    
    counter = 0;
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
                opt_yield = 1;
                break;
            case 's':
                sync_opion = optarg[0];
                if (sync_opion == 'm') {
                        if (pthread_mutex_init(&mutex, NULL) != 0){
                            fprintf(stderr, "Error: failed to create the mutex %s\n", strerror(errno));
                            exit(1);
                        }
                }
                break;
            default:
                fprintf(stderr, "Error: Unreognized argument detected %s\n", strerror(errno));
                exit(1);
        }
        c = getopt_long(argc, argv, "", opt, &option_index);
    }
    
    struct timespec begin, end;
    if(clock_gettime(CLOCK_MONOTONIC, &begin)<0){
        fprintf(stderr, "Error: failed to get the starting time %s\n", strerror(errno));
        exit(1);
    }
    
    pthread_t* thread_ids = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    
    if (thread_ids == NULL) {
        fprintf(stderr, "Error: failed to allocate enough memory for all the threads %s\n", strerror(errno));
        exit(1);
    }
    
    for (long i = 0; i < num_threads; i++) {
            if (pthread_create(&thread_ids[i], NULL, &routine, NULL) != 0) {
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
    
    long num_operations = num_threads * num_iterations * 2;
    long long diff_sec = end.tv_sec - begin.tv_sec;
    long long diff_nsec = end.tv_nsec - begin.tv_nsec;
    long long run_time = 1000000000L * diff_sec + diff_nsec;
    long long average_time = run_time/num_operations;
    char* test_name;
    if(opt_yield == 0 && sync_opion == 'm'){
        test_name = "add-m";
    }
    else if(opt_yield == 0 && sync_opion == 's'){
        test_name = "add-s";
    }
    else if(opt_yield == 0 && sync_opion == 'c'){
        test_name = "add-c";
    }
    else if(opt_yield == 1 && sync_opion == 'm'){
        test_name = "add-yield-m";
    }
    else if(opt_yield == 1 && sync_opion == 's'){
        test_name = "add-yield-s";
    }
    else if(opt_yield == 1 && sync_opion == 'c'){
        test_name = "add-yield-c";
    }
    else if(opt_yield == 1){
        test_name = "add-yield-none";
    }
    else{
        test_name = "add-none";
    }
    print_output(test_name,num_threads,num_iterations,num_operations,run_time,average_time,counter);
    
    if (sync_opion == 'm')
        pthread_mutex_destroy(&mutex);
    
    free(thread_ids);
    
    exit(0);
}
