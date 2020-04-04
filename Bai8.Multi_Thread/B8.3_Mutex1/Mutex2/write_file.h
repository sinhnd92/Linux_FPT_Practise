#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

extern pthread_mutex_t mutex;
void Write2File(char *file_name, char *content);