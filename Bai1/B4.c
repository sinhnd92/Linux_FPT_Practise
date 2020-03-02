#include <stdio.h>

#define Max_char 1000

void writeToFile(FILE *file, char *content)
{
	int c = fputs(content, file);
}

void readFromFile(FILE *file)
{
	char str[Max_char];
	while (fgets(str, Max_char, file) != NULL)
	{
		printf("%s", str);
	}
}

int main(int argc, char **argv)
{
	char c;
	int para_num = argc -1;
	int i, idx = 1;
	FILE *fp;
	if(para_num)
	{
		/*Case opening multiple file*/
		for(i=0; i<para_num; i++)
		{
			fp = fopen(argv[idx], "r");
			if(fp == NULL)
				printf("Can not open file %s\n", argv[idx]);
			else
			{
				printf("Open file %s\n", argv[idx]);
				readFromFile(fp);
				++idx;
				fclose(fp);
			}
		}
	}
	else
	{
		printf("Syntax error\n");
	}
	return 0;
}