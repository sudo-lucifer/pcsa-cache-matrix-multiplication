#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<time.h>

#include"mm-mt.h"

#define MAXBUFFER 512

typedef struct cooridinate{
	long rowStart;
	long colStart;
	long rowEnd;
	long colEnd;
	int package_label;
} matrix_partitions_coor;

matrix_partitions_coor jobList[10];
int jobCount = 0;


pthread_mutex_t mutexLock;

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
	// printf("Start multiply\nrowStart: %ld, rowEnd: %ld\ncolStart: %ld, colEnd: %ld\n\n", rowStart,rowEnd,colStart,colEnd);
	// Your code here
	for (long row = rowStart; row < rowEnd; row += blockSize){
		for(long col = colStart; col < (long) colEnd; col += blockSize){
			for (long blockRow = row; blockRow < row + blockSize && blockRow < rowEnd; blockRow++){
				for (long blockCol = col; blockCol < col + blockSize && blockCol < colEnd; blockCol++){
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
	// printf("Thread Exit done multiply\n");
}

// void multiply_2(long rowStart, long colStart, long rowEnd, long colEnd, long blockSize)
// {
// 	for (long row = rowStart; row < rowEnd; row += blockSize){
// 		for(long col = colStart; col < colEnd; col += blockSize){
// 			for (long blockRow = 0; blockRow < rowEnd - rowStart; blockRow++){
// 				for (long blockCol = col; blockCol < col + blockSize && blockCol < SIZEX; blockCol++){
// 					long index = (blockRow * ((long) SIZEX)) + blockCol;
// 					long sum = huge_matrixC[index];
// 					for(long k = row; k < row + blockSize; k++){
// 						long indexA = (blockRow * ((long) SIZEX)) + k;
// 						long indexB = (blockCol * ((long) SIZEX)) + k;
// 						// long indexB = (k * ((long) SIZEX)) + blockCol;
// 						sum += huge_matrixA[indexA] * huge_matrixB[indexB];
// 					}
// 					huge_matrixC[(blockRow * ((long) SIZEX)) + blockCol] = sum;
// 				}
// 			}
// 		}
// 	}
// }

void * pendingState(void * args){
	pthread_mutex_lock(&mutexLock);
	matrix_partitions_coor package = jobList[0];
	for (int i = 0; i < jobCount - 1; i++){
		jobList[i] = jobList[i + 1];
	}
	jobCount--;
	pthread_mutex_unlock(&mutexLock);
	multiply(package.rowStart, package.colStart, package.rowEnd, package.colEnd, 10);
	return NULL;

}

void addJob(long rowStart, long rowEnd, long colStart, long colEnd){
	matrix_partitions_coor package;
	package.rowStart = rowStart;
	package.rowEnd = rowEnd;
	package.colStart = colStart;
	package.colEnd = colEnd;
	jobList[jobCount] = package;
	jobCount++;
	
}


void multiplyThreading(int ThreadNum){
	// pthread_cond_init(&conditionalVal, NULL);
	pthread_mutex_init(&mutexLock, NULL);
	pthread_t threads[ThreadNum];

	long splitXaxis = ((long) SIZEX) / (((long) ThreadNum) / 2);
	long splitYaxis = ((long) SIZEY) / (((long) ThreadNum) / 2);

	addJob(0,splitXaxis,0,splitYaxis);
	addJob(splitXaxis, SIZEX, splitYaxis, SIZEY);
	addJob(splitXaxis, SIZEX, 0, splitYaxis);
	addJob(0, splitXaxis, splitYaxis, SIZEY);

	for (int i = 0; i < ThreadNum; i++){
		if( pthread_create(&threads[i], NULL, &pendingState,NULL) ){
			printf("Fail to initiaize thread\n");
		}
	}

	for (int i = 0; i < ThreadNum; i++){
		pthread_join(threads[i], NULL);
	}
	pthread_mutex_destroy(&mutexLock);
	
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
	// printMatrixC();
	free_all();

	
	return 0;
}
