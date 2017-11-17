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
#include <sys/socket.h>
#include <errno.h>
#include "commands.h"
#include "built_in.h"
#include "signal_handlers.h"


int sig_pid;//use running program+signal
int pid_save;//save background pid
int background=0;
char bg_command[50]="";
char bg_status[50]="";

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};


void handler(int sig);
void server();
void* client(void* command);
static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i){ 
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
    int built_in_pos=is_built_in_command(com->argv[0]);

    if(strcmp(com->argv[com->argc-1],"&")==0){
      background+=1;
      com->argv[com->argc-1]=NULL;
      com->argc=com->argc-1;
   }

	 printf("n_commands :%d\n",n_commands);
	 built_in_pos=is_built_in_command(com->argv[0]);
	 if(n_commands==1){
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

		printf("tmp\n");
       int pid;
       int status=0;
       char str[5][50]={"/usr/local/bin","/usr/bin/","/bin/","/usr/sbin/","/sbin/"};
       char tmp_long_path[5][50];
       char long_path[50];
		 
  	       pid=fork();
   	    if(pid<0){
       		 perror("Cant't fork process!");
      	 }
          else if(pid==0){//child process running
        		 if(execv(com->argv[0],com->argv)==-1){
         	 for(int i=0;i<5;i++){
            	 strcpy(tmp_long_path[i],str[i]);
            	 strcat(tmp_long_path[i],com->argv[0]);
            	 if(access(tmp_long_path[i],F_OK)==0){
               	strcpy(long_path,tmp_long_path[i]);
                break;
            	 }
          	 }
        	   }
            if(background==1){//child process running in background
          	  signal(SIGINT,SIG_IGN);
				  if(pid_save==0){
					  printf("[1]:%d\n",getpid());
				     if(execv(long_path,com->argv)==-1){
				   	 fprintf(stderr,"%s: command not found!\n",com->argv[0]);
					    exit(0);
					  }else{}
			 	  }else{
				 	  printf("background already exist!!: [1]:%d\n",pid_save);
					  exit(0);
				  }
        	   }
         	else{
        	   }
        	   if(execv(com->argv[0],com->argv)==-1){
          	  if(execv(long_path,com->argv)==-1){
             	 fprintf(stderr,"%s: command not found!\n",com->argv[0]);
             		 exit(0);
          	  }
          	  else{
               execv(long_path,com->argv);
           
			 	  }
		  	   }
        	   else{
			     execv(com->argv[0],com->argv);
        	   }
       		}else{//parent process running
          		if(background==1){
					   if(pid_save==0){
							 strcpy(bg_status,"RUNNING");
				   		 signal(SIGCHLD,(void*)handler);
							 pid_save=pid;
					 		 strcpy(bg_command,com->argv[0]);
		       	   }
            		 background=0;
				 		 return 0;
            	}else{
				 		 sig_pid=pid;
						 signal(SIGINT,(void*)catch_sigint);
						 signal(SIGTSTP,(void*)catch_sigtstp);
						 waitpid(sig_pid,&status,0);
					}
            }
		   
       }//big else
	 } else if(n_commands==2){
		server(com);
	 }
  }//n_command>0 fin
	printf("before return 0 %d\n",getpid());
  return 0;
}//function fin

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
void handler(int sig){
	int status;
  
	while((pid_save==waitpid(pid_save,&status,WNOHANG|WUNTRACED))>0){
		if(WIFEXITED(status)>0){
				strcpy(bg_status,"Exit");
		}		
		else if(WIFSIGNALED(status)!=0){
			if((WTERMSIG(status))==2){
			  printf("job ;%d terminated by signal 2\n",pid_save);
			  strcpy(bg_status,"SIGINT EXIT");
			}
		}else if(WIFSTOPPED(status)==1){
			printf("job:%d stopped by signal 20\n",pid_save);
			strcpy(bg_status,"sig STOPPED");
		}
	}
	if((waitpid(pid_save,&status,WNOHANG|WUNTRACED))==0){
		strcpy(bg_status,"RUNNING");
	}
}

void server(struct single_command (*command)[512]){
	int server_sock,client_sock,len,rc;
	int bytes_rec=0;
	struct sockaddr_un server_sockaddr;//in sys/un.h
	struct sockaddr_un client_sockaddr;
	char buf[256];
	int backlog=10;
	struct single_command * com_f=(*command);
	struct single_command * com_s=(*command+1);

	printf("1 %s\n",com_f->argv[0]);
	printf("2 %s\n",com_s->argv[0]);
	memset(&server_sockaddr,0,sizeof(struct sockaddr_un));
	memset(&client_sockaddr,0,sizeof(struct sockaddr_un));

	//create a unix domain stream socket
	server_sock=socket(PF_FILE,SOCK_STREAM,0);//type : use TCP
	if(server_sock==-1){//success: new socket number, fail: -1
		printf("SOCKET ERROR\n");
			exit(1);
	}

	//set up the unix sockaddr structure by using AF_UNIX for the family and giving it a filepath to bind to
	//unlink the file so the bind will succeed, then bind to that file
	server_sockaddr.sun_family=AF_UNIX;
	strcpy(server_sockaddr.sun_path,"/tmp/test_server.dat");
	len=sizeof(server_sockaddr);

	unlink("/tmp/test_server.dat");


	int option=1;
	setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	rc=bind(server_sock,(struct sockaddr *) &server_sockaddr,len);//allocate socket addr
	if(rc==-1){//success:0 fail:-1
		printf("BIND ERROR\n");
		close(server_sock);
		exit(1);
	}

	//Listen for any client socket
	rc=listen(server_sock,backlog);//create queue for socket, size: backlog.use accept()->communication
	if(rc==-1){//success:0 fail:1
		printf("LISTEN ERROR\n");
		close(server_sock);
		exit(1);
	}
	printf("socket listening...\n");

	pthread_t pit;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_create(&pit,&attr,client,(void*)com_f);

//	pthread_join(pit,NULL);
	while(1){
	  int client_sock_addr_size=sizeof(client_sockaddr);	

	//allow connection with accept() when client requests connection 
		client_sock=accept(server_sock,(struct sockaddr*)&client_sockaddr,&client_sock_addr_size);
		if(client_sock==-1){
			printf("ACCEPT ERROR\n");
			close(server_sock);
			close(client_sock);
			exit(1);
			continue;
		}
	
		if(fork()==0){
			dup2(client_sock,0);
			close(client_sock);
			evaluate_command(1,com_s);
			exit(1);
		}
	}
	pthread_exit(0);
	//close the sockets and exit
	close(server_sock);
	close(client_sock);
	return 0;
}

void* client(void *command){
	int client_sock,rc,len;
	struct sockaddr_un server_sockaddr;
	char buf[256];

	struct single_command* com=(struct single_command*)command;

	

	memset(&server_sockaddr,0,sizeof(struct sockaddr_un));
   
	//create a unix domain stream socket
	client_sock=socket(PF_FILE,SOCK_STREAM,0);
	if(client_sock==-1){
			printf("SOCKET ERROR\n");
			exit(1);
	}

	//set up the unix sockaddr structure by using AF_UNIX for the family and giving it a filepath to bind to. 
	//unlink the file so the bind will succeed, then bind to that file
	server_sockaddr.sun_family=AF_UNIX;
	strcpy(server_sockaddr.sun_path,"/tmp/test_server.dat");
	len=sizeof(server_sockaddr);


	//set up the unix sockaddr structure for the server socket and connect to it

  struct single_command tmp[512];
  tmp[0]=*com;

  rc=connect(client_sock,(struct sockaddr*)&server_sockaddr,len);
		if(rc==-1){
			printf("CONNECT ERROR\n");
			close(client_sock);
			exit(1);
		}if(fork()==0){
			printf("SUCCESS!\n");
			close(STDIN_FILENO);
			
			int s=dup(STDOUT_FILENO);
			dup2(client_sock,STDOUT_FILENO);
			printf("test\n");
			evaluate_command(1,&tmp);
			printf("%d\n",getpid());
			fflush(stdout);
			close(STDOUT_FILENO);
			dup2(s,STDOUT_FILENO);
			wait(NULL);
			exit(1);
		close(client_sock);
		}
	pthread_exit(0);

	return 0;
}


