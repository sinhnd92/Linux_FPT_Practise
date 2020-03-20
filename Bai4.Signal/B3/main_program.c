#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void sig_handler(int signo)
{
	if(signo == SIGINT)
	{
		printf("Wake up\n");
	}
}
int main()
{
	signal(SIGINT, sig_handler);
	sleep(1000000);
	printf("After Wake up, This is in the main program\n");
	return 0;
}
