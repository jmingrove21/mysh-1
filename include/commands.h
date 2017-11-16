
#ifndef MYSH_COMMANDS_H_
#define MYSH_COMMANDS_H_


struct single_command
{
  int argc;
  char** argv;
};
int sig_pid;
extern pid_save;
extern char bg_command[50];
extern char bg_status[50];
int evaluate_command(int n_commands, struct single_command (*commands)[512]);

void free_commands(int n_commands, struct single_command (*commands)[512]);

void addjob(struct single_command (*commands)[512]);
#endif // MYSH_COMMANDS_H_
