#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	sigset_t newset, oldset, pendingset;
	int ret;
	ret = sigemptyset(&newset);
	if(ret == -1)
	{
		printf("Can not reset the signal set\n");
		printf("Error %s\n", strerror(errno));
		return -1;
	}
	ret = sigaddset(&newset, SIGINT);
	if(ret == -1)
	{
		printf("Can not add the signal to the set\n");
		return -1;
	}
	/*Register the block signal for the process*/
	ret = sigprocmask(SIG_BLOCK, &newset, &oldset);
	if(ret<0)
	{
		printf("Error register signal: %s\n", strerror(errno));
	}
	else
		printf("Register block signal successfully\n");
	/*Create an infinite loop*/
	while(1);
	return 0;
}