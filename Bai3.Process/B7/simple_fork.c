#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
  pid_t child_pid, _pid;
  child_pid = fork();
  switch(child_pid)
  {
    case -1:
      printf("Fail to create child process\n");
      break;
    case 0:
      _pid = getpid();
      printf("This is child process, with pid = %d\n", _pid);
      while(1);
      break;
    default:
      _pid = getpid();
      printf("This is parent process, with pid = %d\n", _pid);
      while(1);
  }
  return 0;
}
