#include<stdio.h>
#include<signal.h>
#include<unistd.h>

void sig_int_handler(int sig)
{
  if(sig == SIGINT)
  {
    printf("\nThe interrupt signal has just been received\n");
  }
}

int main(int argc, char const *argv[])
{
  if(signal(SIGINT, sig_int_handler) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");
  while(1)
  {
  	//Using sleep function to make sure this program is using less CPU
  	sleep(1);
  }
  return 0;
}

