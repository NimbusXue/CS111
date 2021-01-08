//NAME: JIAXUAN XUE
//EMAIL: nimbusxue1015@g.ucla.edu
//ID: 705142227

#include <unistd.h>
#include <stdlib.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <stdio.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/errno.h>

int period = 1;
int log_flag = 0;
int file= 0;
char scale_option = 'F';
mraa_aio_context temp_sensor;
mraa_gpio_context button;
int stop_report = 0;
char* filename;


void shutdown() {
    time_t raw_time;
    struct tm * info;
    time(&raw_time);
    info = localtime(&raw_time);
    
    fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);
    if(log_flag == 1){
        dprintf(file, "%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);
//        fflush(file);
    }
    
    exit(0);
}



int main(int argc, char* argv[]) {
    struct option opt[] = {
       {"period", required_argument, NULL, 'p'},
       {"scale", required_argument, NULL, 's'},
       {"log", required_argument, NULL, 'l'},
       {0, 0, 0, 0}
    };
    int option_index = 0;
    int c = getopt_long(argc, argv, "", opt, &option_index);
    while(1){
        if(c == -1) break;
        switch(c){
            case 'p':
                period = atoi(optarg);
                break;
            case 'l':
                filename = optarg;
                file = creat(filename, 0666);
                if(file <0){
                    fprintf(stderr, "Error: Unable to create the log file: %s\n", strerror(errno));
                    exit(1);
                }
                log_flag=1;
                break;
            case 's':
                if(optarg[0]=='F'){
                    scale_option = 'F';
                }else if(optarg[0]=='C'){
                    scale_option = 'C';
                }else{
                    fprintf(stderr, "Error: Unrecognized scale option: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "Error: Unreognized argument detected %s\n", strerror(errno));
                exit(1);
        }
        c = getopt_long(argc, argv, "", opt, &option_index);
    }
    
    button = mraa_gpio_init(60);
    if(button == NULL){
           fprintf(stderr, "Error: Failed to initialize button: %s\n", strerror(errno));
           exit(1);
    }
    
    temp_sensor = mraa_aio_init(1);
    if(temp_sensor == NULL){
           fprintf(stderr, "Error: Failed to initialize temperature sensor: %s\n", strerror(errno));
           exit(1);
    }
    
    if(mraa_gpio_dir(button, MRAA_GPIO_IN) != MRAA_SUCCESS){
           fprintf(stderr, "Error: Failed to set up the direction of data flow for button: %s\n", strerror(errno));
           exit(1);
    }
    
    if(mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &shutdown, NULL) != MRAA_SUCCESS){
           fprintf(stderr, "Error: Failed to properly shut down: %s\n", strerror(errno));
           exit(1);
    }
    
    struct pollfd fds;
    fds.fd = STDIN_FILENO;
    fds.events = POLLIN;
    char* input = malloc(1024 * sizeof(char));
    if(input == NULL) {
        fprintf(stderr, "Error:Failed to allocate enough memory for the input buffer: %s\n", strerror(errno));
        exit(1);
    }
    
    time_t start;
    time(&start);
    time_t end;
    time(&end);
    const int B = 4275;
    const int R0 = 100000;
    
    while(1){
        if((difftime(end, start) >= period )& (!stop_report)){
            time(&start);
            int a = mraa_aio_read(temp_sensor);
            float R = 1023.0/a - 1.0;
            R = R0*R;
            float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;
            
            if(scale_option=='F'){
                temperature = temperature*1.8+32;
            }
            
            time_t raw_time;
            struct tm * info;
            time(&raw_time);
            info = localtime(&raw_time);
            
            fprintf(stdout, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec,temperature);
            if(log_flag == 1){
                dprintf(file, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec,temperature);
               
            }
            
        }
        int value = poll(&fds, 1, 0);
        if(value==1) {
            fgets(input, 1024, stdin);
            if(strcmp(input, "SCALE=F\n") == 0){
                    scale_option = 'F';
                    if(log_flag==1){
                        dprintf(file, "%s", input);
                       
                    }
                
            }
            
            else if(strcmp(input, "SCALE=C\n") == 0){
                scale_option = 'C';
                if(log_flag==1){
                    dprintf(file, "%s", input);
//                    fflush(file);
                }
            }
            
            else if(strstr(input, "PERIOD=") != NULL){
                period = atoi(input + 7);
                if(log_flag==1){
                    dprintf(file, "%s", input);
//                    fflush(file);
                }
                
            }
            
            else if(strcmp(input, "STOP\n") == 0){
                stop_report = 1;
                if(log_flag==1){
                    dprintf(file, "%s", input);
//                    fflush(file);
                }
            }
            
            else if(strcmp(input, "START\n") == 0){
                stop_report = 0;
                if(log_flag==1){
                    dprintf(file, "%s", input);
//                    fflush(file);
                }
            }
            
            else if(strstr(input, "LOG") != NULL){
                if(log_flag==1){
                    dprintf(file, "%s", input);
//                    fflush(file);
                }
            }
            
            else if(strcmp(input, "OFF\n") == 0){
                if(log_flag==1){
                    dprintf(file, "%s", input);
//                    fflush(file);
                }
                shutdown();
            }
            else{
                fprintf(stderr, "Error: Unrecognized command: %s\n", strerror(errno));
                exit(1);
                
            }
            
           
            
        }
        
        time(&end);
        
    }
    
    mraa_aio_close(temp_sensor);
    mraa_gpio_close(button);
    free(input);
    exit(0);
}
