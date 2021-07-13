#define _OPEN_SYS_ITOA_EXT
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include "mm.h"

// Collaborator List:
// 	- Thanawin Boonpojanasoontorn 6280163
// 	- Vanessa Rujipatanakul 6280204
// 	- Krittin Nisunarat 6280782

// Task 1: Flush the cache so that we can do our measurement :)
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

void load_matrix_base()
{
	long i;
	huge_matrixA = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);
	huge_matrixB = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);
	huge_matrixC = malloc(sizeof(long)*(long)SIZEX*(long)SIZEY);
	// Load the input
	// Note: This is suboptimal because each of these loads can be done in parallel.
	for(i=0;i<((long)SIZEX*(long)SIZEY);i++)
	{
		fscanf(fin1,"%ld", (huge_matrixA+i)); 		
		fscanf(fin2,"%ld", (huge_matrixB+i)); 		
		huge_matrixC[i] = 0;		
	}
}

void free_all()
{
	free(huge_matrixA);
	free(huge_matrixB);
	free(huge_matrixC);
}

void multiply_base()
{
	// Your code here
	//
	// Implement your baseline matrix multiply here.
	for (long row = 0; row < (long) SIZEX; row++){
		for(long col = 0; col < (long) SIZEY; col++){
			long sum = 0;
			for (long k = 0; k < (long) SIZEY; k++){
				sum += (huge_matrixA[(row * (long) SIZEY) + k] * huge_matrixB[(k * (long) SIZEY) + col]); 
			}
			huge_matrixC[(row * (long) SIZEX) + col] = sum;
		}
	}
	
}

void compare_results()
{
	// fout = fopen("./out.in","r");
	printf("============ Start Testing ============\n");
	FILE * fref = fopen("./reference.in", "r");
	long i;
	long temp1, temp2;
	for(i=0;i<((long)SIZEX*(long)SIZEY);i++)
	{
		// fscanf(fout, "%ld", &temp1);
		fscanf(fref, "%ld", &temp2);
		temp1 = huge_matrixC[i];
		if(temp1!=temp2)
		{
			printf("Wrong solution!");
			exit(1);
		}
	}
	// fclose(fout);
	fclose(fref);
	printf("Correct Result!!\n");
}

void write_results()
{
	// Your code here
	//
	// Basically, make sure the result is written on fout
	// Each line represent value in the X-dimension of your matrix
	// fout = fopen("./out.in", "r");
	// for (long i = 0; i < (long) SIZEX * (long) SIZEY; i++){
		
		
	// }
	
}

void printMatrixA(){
	for (long i = 0; i < (long)SIZEX * (long)SIZEY; i++)
	{
		if (i % (long)SIZEX == 0)
		{
			printf("\n");
		}
		printf("%ld ", huge_matrixA[i]);
		// if (i == 10){ break; }
	}
	printf("\n");
}

void printMatrixB(){
	for (long i = 0; i < (long)SIZEX * (long)SIZEY; i++)
	{
		if (i % (long)SIZEX == 0)
		{
			printf("\n");
		}
		printf("%ld ", huge_matrixB[i]);
	}
	printf("\n");
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
		}
	}
}


/* cache calculation
	L1:
		262,144 bytes for L1d  = 262,144 / 8 = 32,768 blocks 
		Case two ways: 32,768 / 2 = 16384 sets
			Set Index: log_2 16384 = 14 bit
			BIB: log_2 64 = 8
			Tag: 64 - (14 + 8) = 42 bits
		Case four ways: 32,768 / 4 = 8192 sets
	L2:
		2,097,152 bytes = 2,097,152 / 8 = 262,144 blocks
		Case two ways: 262,144 / 2 = 131,072 sets
		Case four ways: 262,144 / 4 = 65,536 sets
	

*/
	
void multiply()
{
	long blockSize = 100;
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
}

int main()
{
	
	clock_t s,t;
	double total_in_base = 0.0;
	double total_in_your = 0.0;
	double total_mul_base = 0.0;
	double total_mul_your = 0.0;
	fin1 = fopen("./input1.in","r");
	fin2 = fopen("./input2.in","r");
	fout = fopen("./out.in","w");
	ftest = fopen("./reference.in","r");
	

	// flush_all_caches();

	s = clock();
	load_matrix_base();
	t = clock();
	total_in_base += ((double)t-(double)s) / CLOCKS_PER_SEC;
	printf("[Baseline] Total time taken during the load = %f seconds\n", total_in_base);

	s = clock();
	multiply_base();
	t = clock();
	total_mul_base += ((double)t-(double)s) / CLOCKS_PER_SEC;
	printf("[Baseline] Total time taken during the multiply = %f seconds\n", total_mul_base);
	flush_all_caches();
	free_all();

	// return 0;
	fin1 = fopen("./input1.in","r");
	fin2 = fopen("./input2.in","r");
	fout = fopen("./out.in","w");
	ftest = fopen("./reference.in","r");


	s = clock();
	load_matrix();
	t = clock();
	total_in_your += ((double)t-(double)s) / CLOCKS_PER_SEC;
	printf("Total time taken during the load = %f seconds\n", total_in_your);

	clock_t s1 = clock();
	multiply();
	clock_t t1 = clock();
	total_mul_your += ((double)t1-(double)s1) / CLOCKS_PER_SEC;
	printf("Total time taken during the multiply = %f seconds\n", total_mul_your);
	// write_results();
	fclose(fin1);
	fclose(fin2);
	fclose(fout);
	// free_all();
	compare_results();
	flush_all_caches();
	free_all();

	return 0;
}
