#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define LEN 1024

int main()
{
	/*Program that get pid of process*/
	char line[LEN];
	int ret;
	FILE *cmd = popen("pidof main_program", "r");
	fgets(line, LEN, cmd);
	printf("Line of cmd result: %s\n", line);
	pid_t pid = strtoul(line, NULL, 10);
	printf("return pid: %d\n", (int)pid);
	pclose(cmd);
	/*Send signal to the main program*/
	ret = kill(pid, SIGINT);
	if(ret == -1){
		printf("Oh dear, something went wrong: %s\n", strerror(errno));
	}
	else if(ret ==0)
		printf("Sending signal successfully\n");
	return 0;
}
