#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define File_num 10
#define No_Num  5000000

int random_func(int minN, int maxN){
    return minN + rand() % (maxN + 1 - minN);
}

int main(void)
{
  FILE *fp;
  char file_name[20], numc[4];
  int i,j,r;
  srand((int)time(0));
  
  for(i=1; i<=File_num; i++)
  {
    sprintf(file_name, "temp_nonMulti_%d.txt", i);
    fp = fopen(file_name, "a");
    
    for(j=0; j<No_Num; j++)
    {
      r = random_func(1,100);
      sprintf(numc, "%d  ", r);
      fputs(numc, fp);
    }
    fclose(fp);
    printf("Finishing write to file number %d\n", i);
  }
  printf("FINISH WRITE TO 10 FILES\n");
  return 0;
}