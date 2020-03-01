#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MaxRead 100
int main(int argc, char const *argv[])
{
	const char * path = argv[1];
	const char * mode = argv[2];
	char *buf = (char *)malloc(MaxRead + 1);	
	off_t pos_def = atoi(argv[3]);
	int char_number = atoi(argv[4]);
	int n,fd;

	if(argc != 5)
	{
		perror("The number of argument is not correct\n");
		return -1;
	}
	if((mode != "SEEK_SET") && (mode != "SEEK_CUR") && (mode != "SEEK_END") && (char_number < 0))
	{
		perror("Syntax Error\n");
		return -1;
	}	

	fd = open(path, O_RDONLY);
	if(fd == -1){
		perror("Fail to open file\n");
		return -1;
	}
	if(strcmp(mode, "SEEK_SET") == 0)
		lseek(fd, pos_def, SEEK_SET);
	if(strcmp(mode, "SEEK_CUR") == 0)
		lseek(fd, pos_def, SEEK_CUR);
	if(strcmp(mode, "SEEK_END") == 0)
		lseek(fd, pos_def, SEEK_END);
	while((n = read(fd, buf, char_number)) !=0)
	{
		if(n == -1)
		{
			perror("Can't read the file\n");
			return -1;
		}
		if(n<MaxRead)
		{
			buf[n] = '\0';
			puts(buf);
			break;
		}
	}
	close(fd);
	free(buf);
	return 0;
}