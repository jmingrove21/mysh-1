#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "commands.h"
#include "built_in.h"
#include "signal_handlers.h"

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  if (n_commands > 0) {
    struct single_command* com = (*commands);

    assert(com->argc != 0);

    int built_in_pos = is_built_in_command(com->argv[0]);
    if (built_in_pos != -1) {
      if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
	 if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
	 fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } else {
        fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        return -1;
      }
    } else if (strcmp(com->argv[0], "") == 0) {
	 return 0;
    } else if (strcmp(com->argv[0], "exit") == 0) {
	 return 1;
    } else {
     // fprintf(stderr, "%s: command not found\n", com->argv[0]);
     // return -1;
	int status;
    	int background;
	pid_t pid=fork();
	
	if(pid==0)
	{
	  if(execv(com->argv[0],com->argv)==-1){
	   printf(com->argv[0]);
	   if(!strcmp(com->argv[0],"ls")){
            char str[50]="/bin/";
            strcat(str,com->argv[0]);
	    com->argv[0]=str;
            execv(com->argv[0],com->argv);
           }
	   else if(!strcmp(com->argv[0],"vim")){
	   char str[50]="/usr/bin/";
	   strcat(str,com->argv[0]);
	   com->argv[0]=str;
	   execv(com->argv[0],com->argv);
	   }
	   else if(!strcmp(com->argv[0],"cat")){
	   char str[50]="/bin/";
	   strcat(str,com->argv[0]);
	   com->argv[0]=str;
	   execv(com->argv[0],com->argv);
	   }
	  fprintf(stderr,"%s: command not found\n", com->argv[0]);
          exit(0); 
	  }
	}
	else if(pid>0){

	 printf("child:%d\n",(int)getpid());
	 printf("Parent: wait (%d)\n",pid);
 	 waitpid(pid,&status,0);
        }
       }
      }
    
  return 0;
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}

void communication_server(){
 int server_socket;
 int client_socket;
 int client_addr_size;
 int opt;
 int buff_rcv[1029];
 int buff_snd[1029];
 struct sockaddr_un server_addr;
 struct sockaddr_un client_addr;
 
 if(access("/tmp/test_server",F_OK)==0)
 unlink("/tmp/test_server");
 
 server_socket=socket(PF_FILE,SOCK_STREAM,0);
 if(server_socket==-1){
 printf("server socket fail!");
 exit(1);
 }
 memset(&server_addr,0,sizeof(server_addr));
 server_addr.sun_family=AF_UNIX;
 strcpy(server_addr.sun_path,"/tmp/test_server");

 if(-1==bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))){
 printf("binding execution error");
 exit(1);
 }

 while(1){
  if(listen(server_socket,5)==-1){
   printf("waiting mode setting fail\n");
   exit(1);
   }
   client_addr_size=sizeof(client_addr);
   client_socket=accept(server_socket,(struct sockaddr*)&client_addr,&client_addr_size);
  
 if(client_socket==-1){
  printf("FAIL! Can't accept client connection");
  exit(1);
 }

 read(client_socket,buff_rcv,1024);
 printf("receive:%s\n",buff_rcv);
 
 sprintf(buff_snd,"%d:%s", strlen(buff_rcv),buff_rcv);
 write(client_socket,buff_snd,strlen(buff_snd)+1);
 close(client_socket);

 }
}
