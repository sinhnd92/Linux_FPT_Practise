#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define Max_Buf	50
#define Max_Buf_Read	6

bool IsParent = false;
bool parent_running = false;
int counter =0;

int Write2File()
{
	int fd, length, position, idx, jidx, ret;

	char Write_Buf[Max_Buf];
	char Read_Buf[Max_Buf_Read];
	char Counter_Buf[Max_Buf_Read];
	char Init_Buf[] = "The initial counter: 0\n";
	jidx =0;
	/*Open the file text.txt with mode create, read, write*/
	fd = open("text.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if(fd == -1)
	{
		perror("Fail to open text file\n");
	}
	/*Init the first line in text.txt*/
	if (counter == 0)
	{
		write(fd, Init_Buf, sizeof(Init_Buf));
	}
	/*Set the file offset 6 byte from the end of file*/
	position = lseek(fd, -6, SEEK_END);
	/*Read the last 6 byte of file*/
	memset(Read_Buf, 0x0, Max_Buf_Read);
	length = read(fd, Read_Buf, Max_Buf);
	/*Get the value of counter*/
	for(idx = 0; idx < length; idx++)
	{
		if((Read_Buf[idx] <= '9')&&(Read_Buf[idx] >= '0')){
			Counter_Buf[jidx] = Read_Buf[idx];
			jidx++;
		}
		Counter_Buf[jidx] = '\0';
	}
	counter = atoi(Counter_Buf);
	counter++;

	if(IsParent == true)
	{
		memset(Write_Buf, 0x0, Max_Buf);
		length = sprintf(Write_Buf, "This is the parent process, the counter: %d\n", counter);
	}else{
		memset(Write_Buf, 0x0, Max_Buf);
		length = sprintf(Write_Buf, "This is the child process, the counter: %d\n", counter);
	}
	printf("%s\n", Write_Buf);
	/*Set the cursor to the end of file to start writing*/
	position = lseek(fd, 0, SEEK_END);
	ret = write (fd, Write_Buf, length);
	if(ret == -1){
		perror("Fail to write to file\n");
		return -1;
	}
}


void sig_int_handler(int signo)
{
	if(signo == SIGINT){
		if(IsParent){
			parent_running = true;
			printf("Parent Process is continuing writing\n");
		}
		else{
			parent_running = false;
			printf("Child Process is continuing writing\n");
		}
	}
}


int main(int argc, char const *argv[])
{
	int write_number, ret, child_idx, parent_idx;
	int parent_pid, child_pid;
	printf("Input the number of writing pair:\n");
	scanf("%d", &write_number);
	// signal(SIGINT , sig_int_handler);
	if((ret = fork()) < 0)
	{
		perror("Fail to create child process\n");
		return -1;
	}
	else if(ret == 0)
	{
		child_pid = getpid();
		parent_pid = getppid();
		printf("This is child process with pid = %d\n", (int)child_pid);
		parent_running = false;
		for(child_idx = 0; child_idx < write_number; child_idx++)
		{
			IsParent = false;
			/*First the child process send the sigstop to its parent*/
			kill(parent_pid, SIGSTOP);
			/*Write to file*/
			Write2File();
			/*Send sigcont to the parent*/
			kill(parent_pid, SIGCONT);
			kill(parent_pid, SIGINT);
			sleep(1000);
		}
			kill(parent_pid, SIGCHLD);
			return 0;
	}
	else
	{
		child_pid = ret;
		parent_pid = getpid();
		printf("This is parent process with pid = %d\n", (int)parent_pid);
		parent_running = true;
		/*Sleep 1s to make sure the child is running first*/
		sleep(1);
		for(parent_idx = 0; parent_idx < write_number; parent_idx++)
		{
			IsParent = true;
			/*First the process send the sigstop to its child*/
			kill(child_pid, SIGSTOP);
			/*Write to file*/
			Write2File();
			/*Send sigcont to the child*/
			kill(child_pid, SIGCONT);
			kill(child_pid, SIGINT);
			sleep(1000);
		}
		return 0;
	}
}