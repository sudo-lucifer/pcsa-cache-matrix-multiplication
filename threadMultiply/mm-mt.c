#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<time.h>

#include"mm-mt.h"

#define MAXBUFFER 512

typedef struct cooridinate{
	int rowStart;
	int colStart;
	int rowEnd;
	int colEnd;
} matrix_partitions_coor;

matrix_partitions_coor jobList[10];
int jobCount = 0;

pthread_mutex_t mutexLock;
pthread_cond_t conditionalVal;

// optional = task 4
void load_matrix()
{
	// Your code here
	huge_matrixA = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);
	huge_matrixB = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);
	huge_matrixC = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);

	// load with transpose
	for (long row = 0; row < (long) SIZEX; row++){
		for(long col = 0; col < (long)SIZEY; col++){
			// load matrix B to be transpose form
			fscanf(fin2, "%ld", (huge_matrixB + ((col * SIZEY) + row)));
			// load matrix A normally
			fscanf(fin1, "%ld", (huge_matrixA + ((row * SIZEY) + col)));
			huge_matrixC[(row * SIZEY) + col] = 0;		
		}
	}
}

void printMatrixC(){
	for (long i = 0; i < (long)SIZEX * (long)SIZEY; i++)
	{
		if (i % (long)SIZEX == 0)
		{
			printf("\n");
		}
		printf("%ld ", huge_matrixC[i]);
	}
	printf("\n");
}

void write_results()
{
	// Your code here
	//
	// Basically, make sure the result is written on fout
	// Each line represent value in the X-dimension of your matrix
	fout = fopen("./out.in", "w");
	for (long row = 0; row < (long) SIZEX; row++){
		for (long col = 0; col < (long) SIZEY; col++){	
			char buffer[MAXBUFFER];
			sprintf(buffer, "%ld", huge_matrixC[(row * SIZEX) + col]);
			strcat(buffer, " ");
			fwrite(buffer, sizeof(char), strlen(buffer), fout);
			memset(buffer, 0, sizeof(buffer));

		}
		fwrite("\n", sizeof(char), 1, fout);
	}
	fclose(fout);
}

void compare_results()
{
	printf("============ Start Checking Answer ============\n");
	FILE * fref = fopen("./reference.in", "r");
	fout = fopen("./out.in","r");
	long i;
	long temp1, temp2;
	for(i=0;i<((long)SIZEX*(long)SIZEY);i++)
	{
		fscanf(fref, "%ld", &temp2);
		fscanf(fout, "%ld", &temp1);
		if(temp1!=temp2)
		{
			printf("Wrong solution!\n");
			exit(1);
		}
	}
	fclose(fref);
	printf("Correct Result!!\n");
}

void free_all()
{
	free(huge_matrixA);
	free(huge_matrixB);
	free(huge_matrixC);
}

void multiply(long rowStart, long colStart, long rowEnd, long colEnd, long blockSize)
{
	printf("Start multiply\n");
	// Your code here
	for (long row = 0; row < (long) SIZEX; row += blockSize){
		for(long col = 0; col < (long) SIZEY; col += blockSize){
			for (long blockRow = row; blockRow < row + blockSize && blockRow < SIZEX; blockRow++){
				for (long blockCol = col; blockCol < col + blockSize && blockCol < SIZEY; blockCol++){
					long sum = 0;
					for (long k = 0; k < (long)SIZEY; k++)
					{
						// sum += (huge_matrixA[(blockRow * (long)SIZEY) + k] * huge_matrixB[(k * (long)SIZEY) + blockCol]);
						sum += (huge_matrixA[(blockRow * (long)SIZEY) + k] * huge_matrixB[(blockCol * (long)SIZEY) + k]);
					}
					huge_matrixC[(blockRow * (long)SIZEX) + blockCol] = sum;
				}
			}
		}
	}
	printf("Thread Exit done multiply\n");
}

void * pendingState(void * args){
	pthread_detach(pthread_self());
	while (jobCount == 0){
		pthread_cond_wait(&conditionalVal, &mutexLock);
	}
	pthread_mutex_lock(&mutexLock);
	matrix_partitions_coor package = jobList[0];
	for (int i = 0; i < jobCount - 1; i++){
		jobList[i] = jobList[i + 1];
		
	}
	jobCount--;
	pthread_mutex_unlock(&mutexLock);
	multiply(package.rowStart, package.rowEnd, package.colStart, package.colEnd, 10);
	return NULL;

}


void multiplyThreading(int ThreadNum){
	pthread_cond_init(&conditionalVal, NULL);
	pthread_mutex_init(&mutexLock, NULL);
	pthread_t threads[ThreadNum];

	printf("============== Initialize Threads ==============\n");
	for (int i = 0; i < ThreadNum; i++){
		if( pthread_create(&threads[i], NULL, &pendingState,NULL) ){
			printf("Fail to initiaize thread\n");
		}
		// sleep(0.5);
	}
	long splitXaxis = ((long) SIZEX) / (((long) ThreadNum) / 2);
	long splitYaxis = ((long) SIZEY) / (((long) ThreadNum) / 2);

	printf("Adding job\n");
	pthread_mutex_lock(&mutexLock);
	matrix_partitions_coor job1;
	job1.rowStart = 0;
	job1.colStart = 0;
	job1.rowEnd = splitXaxis;
	job1.colEnd = splitYaxis;
	jobList[0] = job1;
	jobCount++;
	pthread_mutex_unlock(&mutexLock);
	pthread_cond_signal(&conditionalVal);

	pthread_mutex_lock(&mutexLock);
	matrix_partitions_coor job2;
	job2.rowStart = splitXaxis;
	job2.colStart = 0;
	job2.rowEnd = SIZEX;
	job2.colEnd = splitYaxis;
	jobList[1] = job2;
	jobCount++;
	pthread_mutex_unlock(&mutexLock);
	pthread_cond_signal(&conditionalVal);

	pthread_mutex_lock(&mutexLock);
	matrix_partitions_coor job3;
	job3.rowStart = 0;
	job3.colStart = splitYaxis;
	job3.rowEnd = splitXaxis;
	job3.colEnd = SIZEY;
	jobList[2] = job3;
	jobCount++;
	pthread_mutex_unlock(&mutexLock);
	pthread_cond_signal(&conditionalVal);
	
	pthread_mutex_lock(&mutexLock);
	matrix_partitions_coor job4;
	job4.rowStart = splitXaxis;
	job4.colStart = splitYaxis;
	job4.rowEnd = SIZEX;
	job4.colEnd = SIZEY;
	jobList[3] = job4;
	jobCount++;
	pthread_mutex_unlock(&mutexLock);
	pthread_cond_signal(&conditionalVal);

	printf("Done Add jobs\n");
	printf("jobCOunt: %d\n", jobCount);
	printf("===============================\n");

	// for (int i = 0; i < ThreadNum; i++){
	// 	pthread_join(threads[i], NULL);
	// }
	pthread_mutex_destroy(&mutexLock);
	pthread_cond_destroy(&conditionalVal);
	
}

int main(){
	clock_t s,t;
	double total_in_base = 0.0;
	double total_in_your = 0.0;
	double total_mul_base = 0.0;
	double total_mul_your = 0.0;
	fin1 = fopen("./input1.in","r");
	fin2 = fopen("./input2.in","r");
	fout = fopen("./out.in","w");
	ftest = fopen("./reference.in","r");

	s = clock();
	load_matrix();
	t = clock();
	total_in_base += ((double)t-(double)s) / CLOCKS_PER_SEC;
	printf("Total time taken during the load = %f seconds\n", total_in_base);

	s = clock();
	multiplyThreading(4);
	t = clock();
	total_in_base += ((double)t-(double)s) / CLOCKS_PER_SEC;
	printf("Total time taken multithread multiply = %f seconds\n", total_in_base);

	fclose(fin1);
	fclose(fin2);
	fclose(fout);
	fclose(ftest);
	printf("============ Start writing the result into file ==========\n");
	write_results();
	printf("Done write\n");
	compare_results();
	free_all();

	
	return 0;
}
