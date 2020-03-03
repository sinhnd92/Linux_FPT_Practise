#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>


int main(int argc, char const *argv[])
{
	DIR *mydir;
	struct dirent *myfile;
	struct stat mystat;
	char mypath[FILENAME_MAX];
	char buf[512];
	if (strcmp(argv[1], "-l") & strcmp(argv[1], "-a") & strcmp(argv[1], "-la") == 0)
		getcwd(mypath, FILENAME_MAX );
	else
		strcpy(mypath, argv[1]);
	mydir = opendir(mypath);
	//Thong bao loi cho 2 truong hop error pho bien la khong duoc quyen truy cap dir va dir khong ton tai
	if(mydir == NULL){
		if(errno == EACCES)
			printf("ls: cannot open directory: Permission denied\n");
		else if(errno == ENOENT)
			printf("ls: cannot access: No such file or directory\n");
	//Neu duong dan chi den 1 file chu khong phai 1 thu muc, in ra thong tin cua file do
		else if(errno == ENOTDIR){
			sprintf(buf, "%s", mypath);
			int res = stat(buf,&mystat);
			if(res != -1)
			{
				printf("%jd", mystat.st_size);
				printf("	%s\n", mypath);
			}
		}
		else
			printf("ls: cannot access the specifield directory\n");	//Thong bao loi cho tat ca truong hop error con lai
	return;
	}
	if (strcmp(argv[1], "-l") & strcmp(argv[1], "-a") & strcmp(argv[1], "-la") == 0)
	{
		while((myfile = readdir(mydir)) != NULL)
		{
			sprintf(buf, "%s/%s", mypath, myfile->d_name);
			stat(buf,&mystat);
			if (strcmp(argv[1], "-l") == 0)
			{
				if (myfile->d_name[0] == '.')
					continue;
					printf("%d", mystat.st_size);
					printf("	%s\n", myfile->d_name);
			}
	    if (strcmp(argv[1], "-a") == 0)
			{
				if(myfile->d_name[0] == '.'){
					printf("%d", mystat.st_size);
					printf("	%s\n", myfile->d_name);
				}
			}
	    if (strcmp(argv[1], "-la") == 0)
			{
					printf("%d", mystat.st_size);
					printf("	%s\n", myfile->d_name);
			}

		}
	}
	else
	{
		while((myfile = readdir(mydir)) != NULL)
		{
			sprintf(buf, "%s/%s", mypath, myfile->d_name);
			stat(buf,&mystat);
			if (strcmp(argv[2], "-l") == 0)
			{
				if (myfile->d_name[0] == '.')
					continue;
					printf("%d", mystat.st_size);
					printf("	%s\n", myfile->d_name);
			}
	    if (strcmp(argv[2], "-a") == 0)
			{
				if(myfile->d_name[0] == '.'){
					printf("%d", mystat.st_size);
					printf("	%s\n", myfile->d_name);
				}
			}
	    if (strcmp(argv[2], "-la") == 0)
			{
					printf("%d", mystat.st_size);
					printf("	%s\n", myfile->d_name);
			}

		}
	}
	closedir(mydir);
}
