#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "write_file.h"

#define Thread_num 2

int random_func(int minN, int maxN){
    return minN + rand() % (maxN + 1 - minN);
}

void *Write_File_Thread(void* argv)
{
  char *file_name = "temp.txt";
  char *content[2];
  content[0] = "Hello, I'm thread number 0\n";
  content[1] = "And I'm thread number 1\n";

  Write2File(file_name, content[*(int *)argv]);

  printf("Finishing write to file by thread number %d\n", *(int *)argv);
  pthread_exit(NULL);
}

int main(void)
{
  int thread_index, ret;
  pthread_t threadID;
  for(thread_index=0; thread_index<Thread_num; thread_index++)
  {
		ret = pthread_create(&threadID, NULL, Write_File_Thread, &thread_index);
		if(ret)
		{
			printf("Fail to creat thread number %d \n", thread_index);
			break;
		}
    pthread_join(threadID, NULL);    
  }
  printf("FINISH WRITE TO FILES\n");
  return 0;
}