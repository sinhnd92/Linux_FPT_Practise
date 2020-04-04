#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#define MAX 10000000000
#define Thread_Num 100
#define MaxPerThread MAX/Thread_Num

uint64_t count = 1;

void *Count_Thread(void* argv)
{
	uint32_t i;
	for(i=0; i<MaxPerThread; i++);
		count++;
	printf("Thread number %d counting finish\n", *(int *)argv);
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t threadID;
	int ret, thread_index;
	for(thread_index=1; thread_index<=Thread_Num; thread_index++)
	{
		ret = pthread_create(&threadID, NULL, Count_Thread, &thread_index);
		if(ret)
		{
			printf("Fail to creat thread number %d \n", thread_index);
			break;
		}
		pthread_join(threadID, NULL);
	}
	printf("Count finish = %" PRId64  "\n", count);
	return 0;
}
