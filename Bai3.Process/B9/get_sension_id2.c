#include <stdio.h>
#include <unistd.h>

int main(){
	
	printf("sension id  of 2 %d " ,getsid(0));
	return 0;
}
