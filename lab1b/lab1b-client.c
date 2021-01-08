//NAME: JIAXUAN XUE
//EMAIL: nimbusxue1015@g.ucla.edu
//ID: 705142227

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <string.h>
#include <ulimit.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <zlib.h>

#define CONTROL_D 0x04
#define CONTROL_C 0x03
//save original terminal modes for restoration
struct termios restoration;
z_stream out_stream; //client to server
z_stream in_stream; //server to client
int sockfd;
int logfd;
char send_prefix[15] = "SENT ";
char receive_prefix[15] = "RECEIVED ";
char unit[15] = " bytes: ";
char newline = '\n';
int log_status;

int create_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock< 0)
    {
      fprintf(stderr, "Error: Failed to create the socket\n");
      exit(1);
    }
    return sock;
}
void compress_data(int read_amount, char * input_buffer, char * compress_buffer) {

  out_stream.avail_in = read_amount;
  out_stream.next_in = (unsigned char *) input_buffer;
  out_stream.avail_out = 300;
  out_stream.next_out = (unsigned char *) compress_buffer;
    
  do{
      deflate(&out_stream,Z_SYNC_FLUSH);
  }while (out_stream.avail_in > 0);
    
}

void decompress_data(int read_amount, char * input_buffer, char * decompress_buffer) {

  in_stream.avail_in = read_amount;
  in_stream.next_in = (unsigned char *) input_buffer;
  in_stream.avail_out = 300;
  in_stream.next_out = (unsigned char *) decompress_buffer;
    
  do{
      inflate(&in_stream,Z_SYNC_FLUSH);
  }while (in_stream.avail_in > 0);
    
}



void restore_terminal(){
    if(tcsetattr(STDIN_FILENO, TCSANOW, &restoration) < 0)
    {
        fprintf(stderr, "Error: Failed to restore the original terminal: %s\n", strerror(errno));
        exit(1);
    }
    if(log_status == 1){
        close(logfd);
    }
    if(shutdown(sockfd, SHUT_RDWR)<0){
        fprintf(stderr, "Error: Failed to shut down the socket: %s\n", strerror(errno));
        exit(1);
    }
}

void set_terminal(){
    struct termios new_terminal;
    if(tcgetattr(STDIN_FILENO, &restoration)< 0)
    {
        fprintf(stderr, "Error: Failed to get the parameters associated with the original terminal: %s\n", strerror(errno));
        exit(1);
    }
    if(tcgetattr(STDIN_FILENO, &new_terminal)< 0)
    {
        fprintf(stderr, "Error: Failed to get the parameters associated with the new terminal: %s\n", strerror(errno));
        exit(1);
    }
    //non-canonical input mode with no echo
    new_terminal = restoration;
    new_terminal.c_iflag = ISTRIP;
    new_terminal.c_oflag = 0;
    new_terminal.c_lflag = 0;
    
    if ((tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal)) < 0){
        fprintf(stderr, "Error: Failed to set the parameters associated with the new terminal %s\n", strerror(errno));
           exit(1);
    }
    
    atexit(restore_terminal);
}



int main(int argc, char * argv[]) {
    struct option opt[] = {
            {"port", required_argument, NULL, 'p'},
            {"log", required_argument, NULL, 'l'},
            {"compress", no_argument, NULL, 'c'},
            {0, 0, 0, 0}
    };
    

    
    int port_status = 0;
    log_status = 0;
    int compress_status = 0;
    char * file_name = NULL;
    int option_index = 0;
    int port_num = 0;
    int c = getopt_long(argc, argv, "", opt, &option_index);
    //get the option status
    while(1){
        if(c == -1) break;
        switch (c) {
            case 'p':
                port_status = 1;
                port_num = atoi(optarg);
                break;
            case 'l':
                log_status = 1;
                file_name = optarg;
                break;
            case 'c':
                compress_status = 1;
                break;
            default:
                fprintf(stderr, "Error: Unrecognized argument. The correct format is: [--port=port_number --log=filename --compress].\n");
                exit(1);
                break;
        }
        c = getopt_long(argc, argv, "", opt, &option_index);
    }
    
    if (port_status == 0){
        fprintf(stderr, "Error: --port option is required to run the program \n");
        exit(1);
    }
    
    if(log_status == 1){
        struct rlimit limit;
        limit.rlim_cur = 10000;
        limit.rlim_max = 10000;
        if (setrlimit(RLIMIT_FSIZE, &limit) < 0)
          {
            fprintf(stderr, "Error: Failed to set the file limit %s\n", strerror(errno));
            exit(1);
          }
        logfd = creat(file_name,0666);
        if (logfd < 0)
          {
            fprintf(stderr, "Error: Failed to create the log file descriptor %s\n", strerror(errno));
            exit(1);
          }
        
    }
    
    if(compress_status == 1){
        //Initialize zlib streams.
         out_stream.zalloc = Z_NULL;
         out_stream.zfree = Z_NULL;
         out_stream.opaque = Z_NULL;
        
        int deflate_return = deflateInit(&out_stream, Z_DEFAULT_COMPRESSION);
        if (deflate_return != Z_OK){
            fprintf(stderr, "Error: Failed to create to create compression stream %s\n", strerror(errno));
            exit(1);
        }

     
        in_stream.zalloc = Z_NULL;
        in_stream.zfree = Z_NULL;
        in_stream.opaque = Z_NULL;

        int inflate_return = inflateInit(&in_stream);
        if (inflate_return != Z_OK){
            fprintf(stderr, "Error: Failed to create decompression stream %s\n", strerror(errno));
            exit(1);
        }
         
    }
    
    //enter non-canonical input mode with no echo
    set_terminal();

    sockfd = create_socket();
    struct sockaddr_in server_address;
    struct hostent *server;
    server = gethostbyname("localhost");
    
    memset( (void * ) &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy( (char *) &server_address.sin_addr.s_addr, (char *) server->h_addr, server->h_length);
    server_address.sin_port = htons(port_num);
    
    if (connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
      fprintf(stderr, "Error: Failed to connect on client side. %s \n", strerror(errno));
      exit(1);
    }
    
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;
    fds[1].revents = 0;
    int flag = 0;
    while(1){
        if(poll(fds, 2, 0)<0){
            fprintf(stderr, "Error: Failed to call poll. %s \n", strerror(errno));
            exit(1);
        }else{
            if(fds[0].revents & POLLIN){ //pending keyboard input
                char buffer[300];
                ssize_t read_amount = read(STDIN_FILENO, buffer, 300);
                for(int i=0;i<read_amount;i++){
                    char output_char = buffer[i];
                    //check CF and LF
                    if(output_char=='\r' || output_char=='\n'){
                        output_char='\r';
                        write (STDOUT_FILENO, &output_char, 1);
                        output_char='\n';
                        write (STDOUT_FILENO, &output_char, 1);
                    }else{//normal output
                        write (STDOUT_FILENO, &output_char, 1);
                    }
                }//end for loop
                if(compress_status == 1){
                    char compress_buffer[300];
                    compress_data(read_amount, buffer, compress_buffer);
                    write(sockfd, compress_buffer, 300 - out_stream.avail_out);
                    if (log_status==1) {
                        char byte_num[10];
                        sprintf(byte_num, "%d", 300 - out_stream.avail_out);
                        write(logfd, send_prefix, strlen(send_prefix));
                        write(logfd, byte_num, strlen(byte_num));
                        write(logfd, unit, strlen(unit));
                        write(logfd, compress_buffer, 300 - out_stream.avail_out);
                        write(logfd, &newline, 1);
                    }
                }else{
                    write(sockfd, buffer, read_amount);
                    if (log_status==1) {
                        char byte_num[10];
                        sprintf(byte_num, "%ld", read_amount);
                        write(logfd, send_prefix, strlen(send_prefix));
                        write(logfd, byte_num, strlen(byte_num));
                        write(logfd, unit, strlen(unit));
                        write(logfd, buffer, read_amount);
                        write(logfd, &newline, 1);
                    }
                    
                }
            }//end fds[0].revents & POLLIN
            if(fds[0].revents & POLLERR || fds[0].revents & POLLHUP){
                flag=1;
            }
            if(fds[1].revents & POLLIN){// pending input from socket
                char sock_buffer[300];
                ssize_t read_amount = read(sockfd, sock_buffer, 300);
                if(read_amount == 0){
                    break;
                }
                int received_bytenum = read_amount;
                if(compress_status == 1){
                    char decompress_buffer[300];
                    decompress_data(read_amount, sock_buffer, decompress_buffer);
                    int depress_amount = 300 - in_stream.avail_out;
                    received_bytenum = depress_amount;
                    for(int i=0;i<depress_amount;i++){
                        char output_char = decompress_buffer[i];
                        //check CF and LF
                        if(output_char=='\r' || output_char=='\n'){
                            output_char='\r';
                            write (STDOUT_FILENO, &output_char, 1);
                            output_char='\n';
                            write (STDOUT_FILENO, &output_char, 1);
                        }else{//normal output
                            write (STDOUT_FILENO, &output_char, 1);
                        }
                    }
//                    write(STDOUT_FILENO, decompress_buffer, 100 - in_stream.avail_out);
//                    if (log_status==1) {
//                        char byte_num[10];
//                        sprintf(byte_num, "%d", received_bytenum);
//                        write(logfd, receive_prefix, strlen(receive_prefix));
//                        write(logfd, byte_num, strlen(byte_num));
//                        write(logfd, unit, strlen(unit));
//                        write(logfd, decompress_buffer,100 - in_stream.avail_out);
//                        write(logfd, &newline, 1);
//                    }
                    if (log_status==1) {
                        char byte_num[10];
                        sprintf(byte_num, "%ld", read_amount);
                        write(logfd, receive_prefix, strlen(receive_prefix));
                        write(logfd, byte_num, strlen(byte_num));
                        write(logfd, unit, strlen(unit));
                        write(logfd, sock_buffer, read_amount);
                        write(logfd, &newline, 1);
                    }
                    
                }else{// no compression
                    for(int i=0;i<read_amount;i++){
                        char output_char = sock_buffer[i];
                        //check CF and LF
                        if(output_char=='\r' || output_char=='\n'){
                            output_char='\r';
                            write (STDOUT_FILENO, &output_char, 1);
                            output_char='\n';
                            write (STDOUT_FILENO, &output_char, 1);
                        }else{//normal output
                            write (STDOUT_FILENO, &output_char, 1);
                        }
                    }
                    if (log_status==1) {
                        char byte_num[10];
                        sprintf(byte_num, "%d", received_bytenum);
                        write(logfd, receive_prefix, strlen(receive_prefix));
                        write(logfd, byte_num, strlen(byte_num));
                        write(logfd, unit, strlen(unit));
                        write(logfd, sock_buffer, received_bytenum);
                        write(logfd, &newline, 1);
                    }
                    
                }
                
                
            }// end socket input if statement
            if(fds[1].revents & POLLERR || fds[1].revents & POLLHUP){
                flag=1;
            }
            
            if(flag==1){
                break;
            }
            
        }//end else
        
    
    }//end while
    if (compress_status == 1) {
        deflateEnd(&out_stream);
        inflateEnd(&in_stream);
    }
  
    exit(0);

    
}
