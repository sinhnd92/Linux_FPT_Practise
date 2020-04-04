#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX 10000000000

int main(void)
{
	uint64_t count = 1;
	while(count <= MAX)
	{
		count++;
    if(count%10000 == 0)
      printf("Count = %" PRId64  "\n", count);
	}
        printf("Count finish = %" PRId64  "\n", count);

	return 0;
}
