#include <stdio.h>

int main(int argc, char *argv[])
{
	const char *path = "/home/sinhnd92/Desktop/BT_Linux/B1/b2.txt";
	FILE *fp;
	char str[255];
	fp = fopen(path, "r");
	if(!fp)
		printf("Open fail\n");
	else
		printf("Open successfully\n");
	while(fgets(str, 255, fp)!= NULL)
	{
		printf("%s", str);
	}
	fclose(fp);
	return 0;
}
