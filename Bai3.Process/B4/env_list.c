#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[], char* env[])
{
	char* name;
	char* value;
  int i = 0;
	while(1)
	{
	  	name = env[i];
	  	if(name != NULL)
	  	{
			value = getenv(name);
			printf("Environment list: %s\n",name );
	    	i++;
    	}
    	else
    		break;
	}
	return 0;
}