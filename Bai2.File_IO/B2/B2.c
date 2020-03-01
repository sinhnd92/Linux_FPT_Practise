#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

//Chuong trinh mo phong lenh ls -l, chay tren he dieu hanh linux, in ra ten file va dung luong file trong directory dinh truoc
//sinhtv3

int main(void)
{
	char *mypath = "/home/sinh/Downloads";
	DIR *mydir;
	struct dirent *myfile;
	struct stat mystat;

	char buf[512];
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
				printf("%zu", mystat.st_size);
				printf("	%s\n", mypath);
			}				
		}
		else
			printf("ls: cannot access the specifield directory\n");	//Thong bao loi cho tat ca truong hop error con lai
	return;
	}
	
	while((myfile = readdir(mydir)) != NULL)
	{
		sprintf(buf, "%s/%s", mypath, myfile->d_name);
		stat(buf,&mystat);
		//Khong hien thi cac thu muc hidden hoac "." va ".." entry
		if(myfile->d_name[0] == '.')
			continue;	
		printf("%zu", mystat.st_size);
		printf("	%s\n", myfile->d_name);	
	}
	closedir(mydir);
}
