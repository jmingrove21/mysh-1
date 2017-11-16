#include <signal.h>
#include <unistd.h>
#include "signal_handlers.h"
#include "commands.h"
void catch_sigint(int signalNo)
{ 
	 
   printf("\nYour input Ctrl+C\nShell doesn't close.\n");
   signal(SIGINT,SIG_DFL);
	kill(sig_pid,SIGKILL);
  // TODO: File this!
}

void catch_sigtstp(int signalNo)
{
 	 printf("\nYou input Ctrl+Z\n");
	 if(getpid()==tcgetpgrp(STDIN_FILENO)){
 	 signal(SIGTSTP,SIG_DFL);
	 kill(sig_pid,SIGSTOP);
	 }
}
