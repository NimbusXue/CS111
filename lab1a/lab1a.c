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
#define CONTROL_D 0x04
#define CONTROL_C 0x03
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

void restore_terminal(){
    if(tcsetattr(STDIN_FILENO, TCSANOW, &restoration) < 0)
    {
        fprintf(stderr, "Error: Failed to restore the original terminal: %s\n", strerror(errno));
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
            {"shell", required_argument, NULL, 's'},
            {0, 0, 0, 0}
    };
    int option_index = 0;
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
            default:
                fprintf(stderr, "Error: Unrecognized argument. The correct format is: [--shell program].\n");
                exit(1);
                break;
        }
        c = getopt_long(argc, argv, "", opt, &option_index);
    }
    
    if(shell_status){
        //shell option is used
        set_terminal();
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
            fds[0].fd = STDIN_FILENO;
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
                if(fds[0].revents & POLLERR || fds[0].revents & POLLHUP){
                    flag=1;
                }else if(fds[0].revents & POLLIN){
                    char buffer[100];
                    ssize_t read_amount = read(STDIN_FILENO, buffer, 100);
                    for(int i=0;i<read_amount;i++){
                        char output_char = buffer[i];
                        //check CF and LF
                        if(output_char=='\r' || output_char=='\n'){
                            output_char='\r';
                            write (STDOUT_FILENO, &output_char, 1);
                            output_char='\n';
                            write (STDOUT_FILENO, &output_char, 1);
                            write (shell_to_terminal[1], &output_char, 1);
                        }
                        //check ^D
                        else if(output_char==CONTROL_D){
                            close(shell_to_terminal[1]);
                            shell_to_terminal[1]=-1;
                        }else if(output_char==CONTROL_C){//check ^C
//                            char c='Q';
//                            write (STDOUT_FILENO, &c, 1);
                            if ((kill(pid, SIGINT)) < 0){
                               fprintf(stderr, "Error: Failed to send SIGINT to shell. %s\n", strerror(errno));
                               exit(1);
                            }
                        }
                        //normal output
                        else{
                            write (STDOUT_FILENO, &output_char, 1);
                            write (shell_to_terminal[1], &output_char, 1);
                        }
                    }
                }
                if(fds[1].revents & POLLERR || fds[1].revents & POLLHUP){
                    flag=1;
                }else if(fds[1].revents & POLLIN){
                    char buffer[100];
                    ssize_t read_amount = read(terminal_to_shell[0], buffer, 100);
                    for(int i=0;i<read_amount;i++){
                        char output_char = buffer[i];
                        //check LF
                        if(output_char=='\n'){
                            output_char='\r';
                            write (STDOUT_FILENO, &output_char, 1);
                            output_char='\n';
                            write (STDOUT_FILENO, &output_char, 1);
                        }
                        //check ^D
                        else if(output_char==CONTROL_D){
                            flag=1;
                        }
                        //normal output
                        else{
                            write (STDOUT_FILENO, &output_char, 1);
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
            int status=0;
            if (waitpid(pid, &status, 0) < 0)
            {
               fprintf(stderr, "Error: Failed to return the process ID. %s\n", strerror(errno));
               exit(1);
            }
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", status&0xff, status/256);
            exit(0);
        }//parent process
        
        
    }else{
        //no shell option
        set_terminal();
        char buffer[100];
        ssize_t read_amount = read(STDIN_FILENO, buffer, 100);
            while( read_amount > 0)
            {
                for(int i=0;i<read_amount;i++){
                    char output_char = buffer[i];
                    //check CF and LF
                    if(output_char=='\r' || output_char=='\n'){
                        output_char='\r';
                        write (STDOUT_FILENO, &output_char, 1);
                        output_char='\n';
                        write (STDOUT_FILENO, &output_char, 1);
                    }
                    //check ^D
                    else if(output_char==CONTROL_D){
                        exit(0);
                    }
                    //normal output
                    else{
                        write (STDOUT_FILENO, &output_char, 1);
                    }
                }
                read_amount = read(STDIN_FILENO, buffer, 100);
            }
        
        exit(0);
    
    }
    
    
    
    
}
