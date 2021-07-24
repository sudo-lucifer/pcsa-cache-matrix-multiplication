#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<omp.h>
#include<string.h>
#include<time.h>

#include"mm-mt.h"

#define MAXBUFFER 512
#define ThreadNum 4

typedef struct cooridinate{
	long rowStart;
	long colStart;
	long rowEnd;
	long colEnd;
	int package_label;
} matrix_partitions_coor;

typedef struct sumUpCoordinate{
	long blockRow;
	long blockCol;
}sumUpStruct;

typedef struct loadWork{
	int isTransposed;
}loadStruct;
loadStruct loadJob[2];

matrix_partitions_coor jobList[10];
int jobCount = 0;

pthread_mutex_t mutexLock;
pthread_mutex_t loadLock;

void flush_all_caches()
{
	// Your code goes here
	for (long i = 0; i<((long)SIZEX*(long)SIZEY);i++){
		asm volatile ("clflush (%0)\n\t"
				:
				: "r"(huge_matrixA + i)
				: "memory");
		asm volatile ("clflush (%0)\n\t"
				:
				: "r"(huge_matrixB + i)
				: "memory");
		asm volatile ("clflush (%0)\n\t"
				:
				: "r"(huge_matrixC + i)
				: "memory");
	}
	asm volatile ("sfence\n\t"
			:
			:
			: "memory");
}

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

// ========================== parallel load =================
void * load_file(void * args){
	pthread_mutex_lock(&loadLock);
	loadStruct package = loadJob[0];
	loadJob[0] = loadJob[1];
	pthread_mutex_unlock(&loadLock);
	int indexingMethod = package.isTransposed;
	
	for (long row = 0; row < (long) SIZEX; row++){
		for(long col = 0; col < (long) SIZEY; col++){
			if (!indexingMethod){
				fscanf(fin1, "%ld", (huge_matrixA + ((row * SIZEY) + col)));
			}
			else{
				fscanf(fin2, "%ld", (huge_matrixB + ((col * SIZEY) + row)));
				
			}
		}
	}
}

void load_parallel(){
	pthread_mutex_init(&loadLock, NULL);
	huge_matrixA = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);
	huge_matrixB = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);
	huge_matrixC = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);

	pthread_t thread[2];

	loadStruct job1;
	job1.isTransposed = 0;
	loadJob[0] = job1;

	loadStruct job2;
	job1.isTransposed = 1;
	loadJob[1] = job1;

	for (int i = 0; i < 2; i++){
		if(pthread_create(&thread[i], NULL, &load_file, NULL)){
			printf("Fail to create thread\n");
		}
	}

	for (int i = 0; i < 2; i++){
		pthread_join(thread[i],NULL);
	}
	pthread_mutex_destroy(&loadLock);
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


void multiply_2(long rowStart, long colStart, long rowEnd, long colEnd, long blockSize)
{
	long index = 0;
	long sum = 0;
	for (long row = rowStart; row < rowEnd; row += blockSize){
		for(long col = colStart; col < colEnd; col += blockSize){
			for (long block = 0; block < (long) SIZEX; block += blockSize){
				// multiply small block
				for (long blockRow = row; blockRow < row + blockSize && blockRow < SIZEX; blockRow++){
					for (long blockCol = col; blockCol < col + blockSize && blockCol < SIZEX; blockCol++){
						index = (blockRow * ((long) SIZEX)) + blockCol;
						sum = huge_matrixC[index];
						for (long k = block; k < block + blockSize; k++){
							sum += huge_matrixA[(blockRow * ((long)SIZEX)) + k] * huge_matrixB[(blockCol * ((long)SIZEX)) + k];

						}
						huge_matrixC[index] = sum;
					}
				}
			}
		}
	}
}



void * pendingState(void * args){
	pthread_mutex_lock(&mutexLock);
	matrix_partitions_coor package = jobList[0];
	for (int i = 0; i < jobCount - 1; i++){
		jobList[i] = jobList[i + 1];
	}
	jobCount--;
	pthread_mutex_unlock(&mutexLock);
	multiply_2(package.rowStart, package.colStart, package.rowEnd, package.colEnd, 10);
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

void initPackage(){
	long splitXaxis = ((long) SIZEX) / (((long) ThreadNum) / 2);
	long splitYaxis = ((long) SIZEY) / (((long) ThreadNum) / 2);

	addJob(0,splitXaxis,0,splitYaxis);
	addJob(splitXaxis, SIZEX, splitYaxis, SIZEY);
	addJob(splitXaxis, SIZEX, 0, splitYaxis);
	addJob(0, splitXaxis, splitYaxis, SIZEY);
}


void multiplyThreading(){
	pthread_mutex_init(&mutexLock, NULL);
	pthread_t threads[ThreadNum];

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
	fin1 = fopen("./input1.in","r");
	fin2 = fopen("./input2.in","r");
	fout = fopen("./out.in","w");
	ftest = fopen("./reference.in","r");

	s = clock();
	load_parallel();
	t = clock();
	total_in_base += ((double)t-(double)s) / CLOCKS_PER_SEC;
	printf("Total time taken during the load in parallel = %f seconds\n", total_in_base);

	initPackage();

	clock_t s1 = clock();
	// multiply_2(0,0,SIZEX/2,SIZEY/2,2);
	multiplyThreading();
	clock_t t1 = clock();
	printf("Total time taken multithread multiply = %f seconds\n", ((double)t1-(double)s1) / CLOCKS_PER_SEC);

	fclose(fin1);
	fclose(fin2);
	fclose(fout);
	fclose(ftest);
	printf("============ Start writing the result into file ==========\n");
	write_results();
	printf("Done write\n");
	compare_results();
	// printMatrixC();
	flush_all_caches();
	free_all();

	
	return 0;
}
