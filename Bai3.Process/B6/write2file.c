#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
  char *buf1 = "child";
  char *buf2 = "parent";
  int fd;
  int var = 0;
  pid_t child_pid;
  fd = open("file1.txt", O_RDWR | O_CREAT, S_IRWXU);
  if (fd == -1)
    perror("Open fail");
  child_pid = fork();
  switch(child_pid)
  {
    case -1:
      printf("Fail to create child process\n");      
      break;
    case 0:
      printf("This is child process\n");
      write(fd, buf1, sizeof(buf1));
      var = 1;
      break;
    default:
      printf("This is parent process\n");
      write(fd, buf2, sizeof(buf2));
      var = 2;
  }
  printf("%d\n", var);
  close(fd);
  return 0;
}
