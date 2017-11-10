#include <signal.h>
#include "signal_handlers.h"

void catch_sigint(int signalNo)
{ 
  if(signalNo==SIGINT)
   printf("\nYour input Ctrl+C\nShell doesn't close.\n");
   signal(SIGINT,SIG_IGN); 
 // signal(signalNo,SIG_IGN);
  // TODO: File this!
}

void catch_sigtstp(int signalNo)
{ 
  if(signalNo==SIGTSTP)
  printf("\nYou input Ctrl+Z\n");
  signal(SIGTSTP,SIG_IGN);
  // TODO: File this!
}
