//NAME: JIAXUAN XUE
//EMAIL: nimbusxue1015@g.ucla.edu
//ID: 705142227

#include <unistd.h>
#include <stdlib.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int period = 1;
int log_flag = 0;
int file= 0;
char scale_option = 'F';
mraa_aio_context temp_sensor;
int stop_report = 0;
int port_num;
char *filename;
char* id;
char* host_name;
int host_flag=0;
int id_flag=0;
int client;
SSL* ssl;

void shutdown_sensor(){
    if (mraa_aio_close(temp_sensor) != MRAA_SUCCESS){
         fprintf(stderr, "Error: failed to shut down the sensor: %s\n", strerror(errno));
         exit(2);
     }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client);
    
}

int main(int argc, char* argv[]) {
    struct option opt[] = {
       {"period", required_argument, NULL, 'p'},
       {"scale", required_argument, NULL, 's'},
       {"log", required_argument, NULL, 'l'},
       {"id", required_argument, NULL, 'i'},
       {"host", required_argument, NULL, 'h'},
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
            case 'i':
                id = optarg;
                id_flag = 1;
                break;
            case 'h':
                host_name = optarg;
                host_flag = 1;
                break;
            default:
                fprintf(stderr, "Error: Unreognized argument detected %s\n", strerror(errno));
                exit(1);
        }
        c = getopt_long(argc, argv, "", opt, &option_index);
    }
    
    struct sockaddr_in sock_address;
    struct hostent* host = gethostbyname(host_name);
    if(log_flag == 0){
        fprintf(stderr, "Error: --log option missing: %s\n", strerror(errno));
        exit(1);
    }
    
    if(host_flag == 0){
        fprintf(stderr, "Error: --host option missing: %s\n", strerror(errno));
        exit(1);
    }
    
    if(id_flag == 0){
        fprintf(stderr, "Error: --id option missing: %s\n", strerror(errno));
        exit(1);
    }
   
    if(strlen(id)!=9){
        fprintf(stderr, "Error: id is not a 9 digit number: %s\n", strerror(errno));
        exit(1);
    }
    
    if (optind < argc) {
            port = atoi(argv[optind]);
            if (port <= 0) {
                fprintf(stderr, "Error: port number must be greater than 0: %s\n", strerror(errno));
                exit(1);
            }
    }else{
        fprintf(stderr, "Error: port number missing: %s\n", strerror(errno));
               exit(1);
    }
    
   
    if (( client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            fprintf(stderr, "Error: failed to set up the  client program.\n");
            exit(2);
    }
    
    if(host==NULL){
          fprintf(stderr, "Error: errors occured when getting host: %s\n", strerror(errno));
          exit(2);
    }
    
    bzero((char*)&sock_address, sizeof(struct sockaddr_in));
    bcopy(host->h_addr, (char*)(&sock_address.sin_addr.s_addr), host->h_length);
    sock_address.sin_family = AF_INET;
    sock_address.sin_port = htons(port_num);
    if (connect(client, (struct sockaddr*)&sock_address, sizeof(sock_address)) < 0){
        fprintf(stderr, "Error: failed to connect from the client: %s\n", strerror(errno));
        exit(2);
    }
    
    
   
    

    
    
    temp_sensor = mraa_aio_init(1);
    if(temp_sensor == NULL){
           fprintf(stderr, "Error: Failed to initialize temperature sensor: %s\n", strerror(errno));
           exit(1);
    }
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    const SSL_METHOD* method = TLSv1_client_method();
    SSL_CTX* new_context = SSL_CTX_new(method);
    if (new_context==NULL){
            fprintf(stderr, "Error: fail to create the SSL context: %s \n", strerror(errno));
            exit(2);
    }
    ssl = SSL_new(new_context);
    if (ssl==NULL){
        fprintf(stderr, "Error: fail to set up the ssl: %s \n", strerror(errno));
        exit(2);
    }
    if (SSL_set_fd(ssl, client) == 0){
        fprintf(stderr, "Error: fail to set fd for SSL: %s \n", strerror(errno));
        exit(2);
    }
    if (SSL_connect(ssl) <= 0)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        fprintf(stderr, "Error: fail to connect to ssl: %s \n", strerror(errno));
        exit(2);
    }
    
    atexit(shutdown_sensor);
    
    char buffer0[30];
    if (sprintf(buffer0, "ID=%d\n", atoi(id)) < 0){
            fprintf(stderr, "Error: failed to write ID into the buffer: %s\n", strerror(errno));
            exit(2);
    }
    if (SSL_write(ssl, buffer0, strlen(buffer0)) <= 0){
            fprintf(stderr, "Error: fail to call ssl write %s \n", strerror(errno));
            exit(2);
    }

    if(log_flag==1){
        if ( write(file, buffer0, strlen(buffer0)) < 0){
                fprintf(stderr, "Error: failed to write to log file: %s\n", strerror(errno));
                exit(2);
        }
    }

  
    
    struct pollfd fds;
    fds.fd = client;
    fds.events = POLLIN;
    char* input = malloc(1024 * sizeof(char));
    char* buffer2 = malloc(1024 * sizeof(char));
    if(input == NULL) {
        fprintf(stderr, "Error:Failed to allocate enough memory for the input buffer: %s\n", strerror(errno));
        exit(1);
    }
    
    struct timeval start;
    struct timeval end;
    struct tm* info;
    end.tv_sec = 0;
    const int B = 4275;
    const int R0 = 100000;
    int index = 0;
    
    while(1){
        if (gettimeofday(&start, NULL) < 0){
            fprintf(stderr, "Error: failed to get time of the day: %s\n", strerror(errno));
                   exit(2);
        }
        int a = mraa_aio_read(temp_sensor);
        if(a<0){
            fprintf(stderr, "Error: failed to read the temperature: %s\n", strerror(errno));
                    exit(2);
        }
        float R = 1023.0/a - 1.0;
        R = R0*R;
        float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;
        
        if(scale_option=='F'){
            temperature = temperature*1.8+32;
        }
        if((end.tv_sec <= start.tv_sec) && (!stop_report)){
            info = localtime(&start.tv_sec);
            if(info == NULL){
                fprintf(stderr, "Error: failed to get the local time: %s\n", strerror(errno));
                                exit(2);
            }
            if (sprintf(buffer2, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec,temperature) < 0){
                fprintf(stderr, "Error: failed to write to buffer: %s\n", strerror(errno));
                            exit(2);
            }
            
            if (SSL_write(ssl, buffer2, strlen(buffer2)) <= 0){
                    fprintf(stderr, "Error: fail to call ssl write %s \n", strerror(errno));
                    exit(2);
            }

            if(log_flag==1){
                if ( write(file, buffer2, strlen(buffer2)) < 0){
                        fprintf(stderr, "Error: failed to write to log file: %s\n", strerror(errno));
                        exit(2);
                }
            }

            end = start;
            end.tv_sec += period;
        }
        int value = poll(&fds, 1, 0);
        if(value > 0) {
            if (fds.revents & POLLERR){
                fprintf(stderr, "Error: failed to read stdin: %s\n", strerror(errno));
                exit(2);
            }
            if (fds.revents & POLLIN){
                int read_amount = SSL_read(ssl, buffer2, 1024);
                if(read_amount<0){
                    fprintf(stderr, "Error: failed to read ssl: %s\n", strerror(errno));
                        exit(2);
                }
                for (int i = 0; i < read_amount; i++){
                    char c = buffer2[i];
                    if(c=='\n'){
                        input[index]='\0';
                        if(log_flag==1){
                            char temp[1024];
                            strcpy(temp, input);
                            int len =strlen(temp);
                            temp[len] = '\n';
                            if (write(file, temp, strlen(input) + 1) < 0){
                                fprintf(stderr, "Error: failed to write to log: %s\n", strerror(errno));
                                exit(2);
                            }
                        }
                        if(strcmp(input, "START")==0){
                            stop_report = 0;
                        }else if(strcmp(input, "STOP")==0){
                            stop_report = 1;
                        }else if(strncmp(input, "SCALE=F", 7)==0){
                            scale_option = 'F';
                        }else if(strncmp(input, "SCALE=C", 7)==0){
                            scale_option = 'C';
                        }else if(strncmp(input, "PERIOD=", 7)==0){
                            period = atoi(input+7);
                        }else if(strncmp(input, "LOG", 3)!=0){
                            fprintf(stderr, "Error: log option is mandatory \n");
                            exit(1);
                        }else if(strncmp(input, "OFF")==0){
                            struct tm* local;
                            struct timeval now;
                            if (gettimeofday(&now, NULL) < 0){
                                    fprintf(stderr, "Error: failed to get time of the day: %s \n", strerror(errno));
                                    exit(2);
                            }
                            local = localtime(&now.tv_sec);
                            if(local == NULL){
                                fprintf(stderr, "Error: failed to get the local time: %s \n", strerror(errno));
                                        exit(2);
                                
                            }
                            char buffer3[1024];
                            if (sprintf(buffer3, "%02d:%02d:%02d SHUTDOWN\n", local->tm_hour, local->tm_min, local->tm_sec) < 0){
                                fprintf(stderr, "Error: failed to write to buffer: %s\n", strerror(errno));
                                            exit(2);
                            }
                            
                            if (SSL_write(ssl, buffer3, strlen(buffer3)) <= 0){
                                    fprintf(stderr, "Error: fail to call ssl write %s \n", strerror(errno));
                                    exit(2);
                            }

                            if(log_flag==1){
                                if ( write(file, buffer3, strlen(buffer3)) < 0){
                                        fprintf(stderr, "Error: failed to write to log file: %s\n", strerror(errno));
                                        exit(2);
                                }
                            }
                            exit(0);
                        }
                        
                        index = 0;
                        
                    }else{
                        input[index]= buffer2[i];
                        index++;
                    }
                }
            }
        }
    } //while
    
    
    free(input);
    free(buffer2);
    exit(0);
}
