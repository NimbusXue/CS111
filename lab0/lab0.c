//NAME: JIAXUAN XUE
//EMAIL: nimbusxue1015@g.ucla.edu
//ID: 705142227

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

void handle_sigsegv(int sig){
    //just to remove unused warning
    if(sig>0){
        sig=SIGSEGV;
    }
    fprintf(stderr,"Error: Segmentation fault caught from signal. %s\n",strerror(errno));
      exit(4);
}

int main(int argc,  char * argv[]) {
    int option_index = 0;
    struct option long_options[] = {
    { "input", required_argument, NULL, 'i'},
    { "output", required_argument, NULL, 'o'},
    { "segfault", no_argument, NULL, 's'},
    { "catch", no_argument, NULL, 'c'},
    { 0, 0, 0, 0 }
    };
    int c = getopt_long(argc, argv, "i:o:sc", long_options, &option_index);
    char* input_file;
    char* output_file;
    int input_status = 0;
    int output_status = 0;
    int segfault_status = 0;
    int catch_status = 0;
    
    while(1){
        if(c == -1) break;
        switch (c) {
            case 'i':
                input_file = optarg;
                input_status = 1;
                break;
            case 'o':
                output_file = optarg;
                output_status = 1;
                break;
            case 's':
                segfault_status = 1;
                break;
            case 'c':
                catch_status = 1;
                break;
            default:
                fprintf(stderr, "Error: Unrecognized argument. The correct format is: [--input filename] [--output filename] [--segfault] [--catch] \n");
                exit(1);
                break;
        }
        c = getopt_long(argc, argv, "i:o:sc", long_options, &option_index);
    }
    
//    do any file redirection
    if (input_status){
        int ifd = open(input_file, O_RDONLY);
        if (ifd >= 0) {
            close(0);
            dup(ifd);
            close(ifd);
        }else{
            fprintf(stderr, "Error: Unable to open the specified input file %s. %s\n", input_file, strerror(errno));
            exit(2);
        }
    }
    
//    do any file redirection
    if (output_status){
        int ofd = creat(output_file, 0666);
        if (ofd >= 0) {
            close(1);
            dup(ofd);
            close(ofd);
        }else{
            fprintf(stderr, "Error: Unable to create the specified output file %s. %s\n", output_file, strerror(errno));
            exit(3);
        }
    }
//    register the signal handler
    
    if(catch_status){
        signal(SIGSEGV, handle_sigsegv);
    }
//    cause the segfault
    if(segfault_status){
        char* seg=NULL;
        *seg = 1;
    }
    
//    if no segfault was caused, copy stdin to stdout
    char buffer[200];
    ssize_t read_amount = read(0, buffer, 200);
        while( read_amount > 0)
        {
            write (1, buffer, read_amount);
            read_amount = read(0, buffer, 200);
        }
    
    exit(0);
    
}
