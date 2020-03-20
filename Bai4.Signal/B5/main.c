#include <stdio.h> 
#include <signal.h> 

void print_sigset_t(sigset_t *set)
{
	int i;

	i = SIGRTMAX;
	do {
		int x = 0;
		i -= 4;
		if (sigismember(set, i+1)) x |= 1;
		if (sigismember(set, i+2)) x |= 2;
		if (sigismember(set, i+3)) x |= 4;
		if (sigismember(set, i+4)) x |= 8;
		printf("%x", x);
	} while (i >= 4); 
	printf("\n");
}

int main(int argc, char **argv)
{
	sigset_t set;
	sigemptyset(&set);
	/* add signals to set */
	sigaddset(&set, SIGINT); 
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGALRM);
	/* dump signals from set*/
	print_sigset_t(&set);
}