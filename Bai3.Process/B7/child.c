#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int fd;
	ssize_t ret = -1;
	char *write_buffer = "Something\n";
	if(argv[0] == NULL){
		printf("the argument is invalid\n");
		return -1;
	}else{
		fd = open(argv[0], O_RDWR| O_CREAT| O_APPEND, S_IRUSR | S_IWUSR);
		ret = write(fd, write_buffer, 10);
		close(fd);
		printf("File description: %d\n", ret);
		if(ret>=0)
			return 0;
		else
			return -1;
	}
	
}
