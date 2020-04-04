#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#define File_num 10
#define No_Num  5000000

int random_func(int minN, int maxN){
    return minN + rand() % (maxN + 1 - minN);
}

void *Write_File_Thread(void* argv)
{
  FILE *fp;
  char file_name[20], numc[4];
  int j,r;
  srand((int)time(0));
    
  sprintf(file_name, "temp_Multi_%d.txt", *(int *)argv);
  fp = fopen(file_name, "a");
  
  for(j=0; j<No_Num; j++)
  {
    r = random_func(1,100);
    sprintf(numc, "%d  ", r);
    fputs(numc, fp);
  }
  fclose(fp);
  printf("Finishing write to file number %d\n", *(int *)argv);
	pthread_exit(NULL);
}

int main(void)
{
  int thread_index, ret;
  pthread_t threadID;
  for(thread_index=0; thread_index<File_num; thread_index++)
  {
		ret = pthread_create(&threadID, NULL, Write_File_Thread, &thread_index);
		if(ret)
		{
			printf("Fail to creat thread number %d \n", thread_index);
			break;
		}
    pthread_join(threadID, NULL);    
  }
  printf("FINISH WRITE TO 10 FILES\n");
  return 0;
}