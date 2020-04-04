#include "write_file.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void Write2File(char *file_name, char *content)
{
	FILE *fp;
	pthread_mutex_lock(&mutex);
	fp = fopen(file_name, "a");
    fputs(content, fp);
    fclose(fp);
    pthread_mutex_unlock(&mutex);
}