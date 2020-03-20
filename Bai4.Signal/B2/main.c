#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signo)
{
  printf("Wake up\n");
}
int main()
{
  signal(SIGINT, sig_handler);
  unsigned int retval = sleep(10000);
  printf("Sleep for %ds\n", 10000 - retval);
  printf("Good morning\n");
  return 0;
}