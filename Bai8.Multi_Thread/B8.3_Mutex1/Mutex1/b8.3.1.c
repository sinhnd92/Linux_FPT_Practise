#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

#define Thread_num 5
pthread_t thread_ID[Thread_num];
int count = 2020;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *Count_Thread(void *argv)
{
  pthread_mutex_lock(&mutex);
  count++;
  printf("Thread %d finished, count = %d\n", *(int *)argv, count);
  pthread_mutex_unlock(&mutex);
  pthread_exit(NULL);
}

int main(void)
{
  
  int i=0, ret = 0;
  for(i=0; i<Thread_num; i++)
  {
    ret = pthread_create(&thread_ID[i], NULL, Count_Thread, &i);
		if (ret != 0)
		{
			printf("Thread [%d] created error\n", i);
		} 
 	pthread_join(thread_ID[i], NULL);     
  }
  return 0;
}