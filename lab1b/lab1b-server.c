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

z_stream in_stream; //client to server
z_stream out_stream; //server to client
int sockfd;
int new_sockfd;
//save original terminal modes for restoration
struct termios restoration;
int terminal_to_shell[2];
int shell_to_terminal[2];
int pid;

void signal_handler(int sig) {
    //just to remove unused warning
    if(sig>0){
        sig=SIGPIPE;
    }
  fprintf(stderr,"Error: broken pipe\n");

}

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

int main(int argc, char * argv[]) {
    struct option opt[] = {
            {"shell", required_argument, NULL, 's'},
            {"port", required_argument, NULL, 'p'},
            {"compress", no_argument, NULL, 'c'},
            {0, 0, 0, 0}
    };
    int option_index = 0;
    int port_status = 0;
    int compress_status = 0;
    int port_num = 0;
    int c = getopt_long(argc, argv, "", opt, &option_index);
    int shell_status = 0;
    char* input_program;
    //get the shell option status
    while(1){
        if(c == -1) break;
        switch (c) {
            case 's':
                input_program = optarg;
                shell_status = 1;
                break;
            case 'p':
                port_status = 1;
                port_num = atoi(optarg);
                break;
            case 'c':
                compress_status = 1;
                break;
            default:
                fprintf(stderr, "Error: Unrecognized argument. The correct format is: [--shell program --port=port_number --compress].\n");
                exit(1);
                break;
        }
        c = getopt_long(argc, argv, "", opt, &option_index);
    }
    
    if (port_status == 0){
        fprintf(stderr, "Error: --port option is required to run the program \n");
        exit(1);
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
    
    //create socket
    sockfd = create_socket();
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
//    struct hostent *server;
    
    memset( (void * ) &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port_num);
    
    if (bind(sockfd, (struct sockaddr * ) &server_address, sizeof(server_address)) < 0)
      {
        fprintf(stderr, "Error: Failed to bind to the socket. %s\n", strerror(errno));
        exit(1);
      }
    
    if (listen(sockfd, 5) < 0)
      {
        fprintf(stderr, "Error: Failed to listen for incoming connections. %s\n", strerror(errno));
        exit(1);
      }
    
    unsigned int client_address_length = sizeof(client_address);
    new_sockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_address_length);
    if(new_sockfd<0){
        fprintf(stderr, "Error: Failed to accept a connection on a socket. %s\n", strerror(errno));
        exit(1);
    }
    
    
    if(shell_status){

        //create two pipes
        if ((pipe(terminal_to_shell)) < 0)
         {
             fprintf(stderr, "Error: Failed to create pipe from terminal to shell. %s\n", strerror(errno));
             exit(1);
         }
        if ((pipe(shell_to_terminal)) < 0)
         {
             fprintf(stderr, "Error: Failed to create pipe from shell to terminal. %s\n", strerror(errno));
             exit(1);
         }
        signal(SIGPIPE, signal_handler);
        pid = fork();
        if(pid < 0){
            fprintf(stderr, "Error: Failed to create a new process. %s\n",strerror(errno));
            exit(1);
        }else if(pid==0){//child process
            dup2(shell_to_terminal[0], STDIN_FILENO);
            dup2(terminal_to_shell[1], STDOUT_FILENO);
            dup2(terminal_to_shell[1], STDERR_FILENO);
            close(shell_to_terminal[1]);
            close(terminal_to_shell[0]);
            close(terminal_to_shell[1]);
            close(shell_to_terminal[0]);
            if(execl(input_program,input_program, (char*)NULL)==-1){
                fprintf(stderr, "Error: Failed to  exec the specified program. %s\n",strerror(errno));
                exit(1);
            }
        }else{//parent process
            close(terminal_to_shell[1]);
            close(shell_to_terminal[0]);
            struct pollfd fds[2];
            fds[0].fd = new_sockfd;
            fds[0].events = POLLIN;
            fds[0].revents = 0;
            fds[1].fd = terminal_to_shell[0];
            fds[1].events = POLLIN;
            fds[1].revents = 0;
            int flag=0;
            while(1){
                if((poll(fds, 2, 0)) < 0){
                    fprintf(stderr, "Error: Failed to call poll() function. %s\n",strerror(errno));
                    exit(1);
                }
                if(fds[0].revents & POLLERR ){
                    fprintf(stderr, "Error: Failed to poll from socket. %s\n",strerror(errno));
                    exit(1);
                }else if(fds[0].revents & POLLIN){
                    char sock_buffer[300];
                    ssize_t read_amount = read(new_sockfd, sock_buffer, 300);
                    if(compress_status==1){
                        char decompress_buffer[300];
                        decompress_data(read_amount, sock_buffer, decompress_buffer);
                        int depress_amount = 300 - in_stream.avail_out;
                        for(int i=0;i<depress_amount;i++){
                            char output_char = decompress_buffer[i];
                            //check CF and LF
                            if(output_char=='\r' || output_char=='\n'){
                                output_char='\n';
                                write (shell_to_terminal[1], &output_char, 1);
                            } //check ^D
                            else if(output_char==CONTROL_D){
                                close(shell_to_terminal[1]);
                                shell_to_terminal[1]=-1;
                                flag=1;
                            }else if(output_char==CONTROL_C){//check ^C
                                if ((kill(pid, SIGINT)) < 0){
                                   fprintf(stderr, "Error: Failed to send SIGINT to shell. %s\n", strerror(errno));
                                   exit(1);
                                }
                            }else{//normal output
                                write (shell_to_terminal[1], &output_char, 1);
                            }
                        }
                        
                    }else{// end if compress
                        for(int i=0;i<read_amount;i++){
                            char output_char = sock_buffer[i];
                            //check CF and LF
                            if(output_char=='\r' || output_char=='\n'){
                                output_char='\n';
                                write (shell_to_terminal[1], &output_char, 1);
                            } //check ^D
                            else if(output_char==CONTROL_D){
                                close(shell_to_terminal[1]);
                                shell_to_terminal[1]=-1;
                                flag=1;
                            }else if(output_char==CONTROL_C){//check ^C
                                if ((kill(pid, SIGINT)) < 0){
                                   fprintf(stderr, "Error: Failed to send SIGINT to shell. %s\n", strerror(errno));
                                   exit(1);
                                }
                            }else{//normal output
                                write (shell_to_terminal[1], &output_char, 1);
                            }
                        }
                        
                    }
 
                } //end else if fds[0].revents & POLLIN
                if(fds[1].revents & POLLERR || fds[1].revents & POLLHUP){
                    flag=1;
                }else if(fds[1].revents & POLLIN){// pending  shell output
                    char buffer[300];
                    ssize_t read_amount = read(terminal_to_shell[0], buffer, 300);
//                    for(int i=0;i<read_amount;i++){
//                        if(buffer[i]==CONTROL_D){
//                            flag=1;
//                        }
//                    }
                    if(compress_status == 1){
                        char compress_buffer[300];
                        compress_data(read_amount, buffer, compress_buffer);
                        write(new_sockfd, compress_buffer, 300 - out_stream.avail_out);
                    }else{
                        for(int i=0;i<read_amount;i++){
                            char output_char = buffer[i];
                            //check ^D
                            if(output_char==CONTROL_D){
                                flag=1;
                            }
                            //normal output
                            else{
                                write (new_sockfd, &output_char, 1);
                            }
                        }
                        
                    }
                    
                    
                }
                if(flag==1){
                    break;
                }
                
                
            }//while
//            close(terminal_to_shell[1]);
//            close(shell_to_terminal[0]);
            close(terminal_to_shell[0]);
            if(shell_to_terminal[1]!=-1){
                close(shell_to_terminal[1]);
            }
            
            if (compress_status == 1) {
                deflateEnd(&out_stream);
                inflateEnd(&in_stream);
            }
            
            int status=0;
            if (waitpid(pid, &status, 0) < 0)
            {
               fprintf(stderr, "Error: Failed to return the process ID. %s\n", strerror(errno));
               exit(1);
            }
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", status&0xff, status/256);
            close(sockfd);
            close(new_sockfd);
            exit(0);
        }//parent process
        
        
    }
    
    
    
    
}
