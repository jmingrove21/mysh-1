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
  int background=0;

  if (n_commands > 0) {
    struct single_command* com = (*commands);

    assert(com->argc != 0);
  
   int built_in_pos = is_built_in_command(com->argv[0]);
   if(strcmp(com->argv[com->argc-1],"&")==0){
      background+=1;
       com->argv[com->argc-1]=NULL;
  }
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

	pid_t pid=fork();
	
	if(pid==0)
	{
	  if(execv(com->argv[0],com->argv)==-1){
	   char str[5][50]={"/usr/local/bin/","/usr/bin/","/bin/","/usr/sbin/","/sbin/"};
           for(int i=0;i<5;i++){
	    char tmp[50];
	    strcpy(tmp,com->argv[0]);
    	    strcat(str[i],tmp);
	    com->argv[0]=str[i];

	    if(!execv(com->argv[0],com->argv)==-1){
            
            }else if(i==4&&execv(com->argv[0],com->argv)==-1){
	      com->argv[0]=tmp;
	      fprintf(stderr,"%s: command not found!\n",com->argv[0]);
	      exit(0);
	     }else{
	      com->argv[0]=tmp;
	     }
           }
          }
        }
	else if(pid>0){
         printf("status: %d\n",status);
         printf("%d\n",(int)getpid());
         if(WIFEXITED(status)) 
          printf("OK: Child exited with exit status %d.\n",WEXITSTATUS(status));
         else
          printf("ERROR: child has not terminated correctly.\n");
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
void addjob(struct single_command (*commands)[512],pid_t pid, int state){
 struct single_command *com =(*commands);
// if(state



}

void sigchld_handler(int sig){
 int status;
 pid_t cpid;

 while((cpid=waitpid(-1,&status,WNOHANG|WUNTRACED))>0){
 //standard finish
 printf("tmp\n");
 printf("%d\n",WIFEXITED(status));
 if((WIFEXITED(status))>0){
  if(1)
   printf("Normal Terminate");
 }else if((WIFSIGNALED(status))!=0){
   if((WTERMSIG(status)==2)){
   printf("terminated by signal sigint\n");
   }else if((WIFSTOPPED(status))==10){
    printf("Stop by signal sigstsp\n");
   }
  }
 }
}
