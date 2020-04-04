#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#define ROW 200
#define COL 200
#define Matrix_Num	10

int A[ROW][COL];
int B[ROW][COL];
int Result[ROW][COL];
typedef struct Matrix_property{
	int **M;
	int **N;
	int row_idx;
	int col_idx;
}matrix_p;

int random_func(int minN, int maxN){
    return minN + rand() % (maxN + 1 - minN);
}

void SetMatrix(int M[][COL], int i, int j)
{
	int idx_i, idx_j;
	srand((int)time(0));
	for(idx_i = 0; idx_i < i; idx_i++)
		for(idx_j = 0; idx_j < j; ++idx_j)
		{
			M[idx_i][idx_j] = random_func(0,9);
		}
}
void Print_Matrix(int M[][COL], int i, int j)
{
	int idx_i, idx_j;
	for(idx_i = 0; idx_i < i; idx_i++)
	{
		for(idx_j = 0; idx_j < j; ++idx_j)
		{
			printf("%d\t", M[idx_i][idx_j]);
		}
		printf("\n");
	}
}

void *SigleMultiple(void *argv)
{
	matrix_p *mtx = (matrix_p *)argv;
	int sum = 0, i=0;
	for(i=0; i < COL; i++)
		sum += mtx->M[mtx->row_idx][i]*mtx->N[i][mtx->col_idx];
	pthread_exit((void *)&sum);
}

int main(void)
{
	pthread_t threadID[ROW*COL];
	int i, j, ret;
	int **thr_ret;
	SetMatrix(A, ROW, COL);
	SetMatrix(B, ROW, COL);
	for(i=0; i<ROW; i++)
	{
		for(j = 0; j < COL; ++j)
		{
			matrix_p mtrix;
			mtrix.M = A;
			mtrix.N = B;
			mtrix.row_idx = i;
			mtrix.col_idx = j;
			ret = pthread_create(&threadID[i], NULL, SigleMultiple, &mtrix);
			if(ret)
				{
					printf("Fail to creat thread at Result[%d][%d] \n", i, j);
					break;
				}
			pthread_join(threadID[i], (void **)thr_ret);
			Result[i][j] = **thr_ret;
		}
	}
	printf("Result:\n");
	Print_Matrix(Result, ROW, COL);
	return 0;
}