#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
	char *child_path = "/home/sinh/Desktop/Linux_kernel/Linux_FPT_Practise/Bai3.Process/B7/child";
	char *file_path = "/home/sinh/Desktop/Linux_kernel/Linux_FPT_Practise/Bai3.Process/B7/temp.txt";
	int fd,pid,status;
	pid = fork();
	if(pid>=0)
	{
		if(pid >0)
		{
			printf("This is the parent process\n");
			pid = wait(&status);
			if (-1 == pid)
			{
				printf("Error, wait" );
			}
			/*In PID của tiến trình con khi wait() return*/
			printf ("pid=%d\n" , pid);
			printf("status = %d\n", status);
			if(status==-1)
				printf("Fail to write on child process\n");
			if(status == 0)
				printf("write on child process successfully\n");
			
		}else{
			printf("This is the child process\n");
			execl(child_path, file_path, NULL);
		}
	}else{
		printf("Fail to create child process\n");
	}
	return 0;
}
