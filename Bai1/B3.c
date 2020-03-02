#include <stdio.h>

int main(int argc, char **argv){
	char *pid = argv[1];
	char path[50]; 
	char pid_name[30];
	FILE *fp;

	sprintf(path,"/proc/%s/cmdline",pid);
	if((fp = fopen(path, "r")) != NULL)
	{
		fgets(pid_name, 30, fp);
		puts(pid_name);
	}else
	{
		printf("%s: No such file or directory\n", path);
	}
	fclose(fp);
	return 0;
}
