## Principle of Computer and Architecture: Ajarn Rachata Part
Topic: Cache multiplication \
Date: 14 July 2021

# Project detemination
- Optimize matrix generator (mm-gen.py) using lost comprehension
- singleThreadMultiplpication folder = matrix multiply base, line matrix multiplication, tiling matrix multiplication
	- Implement load matrix A normally, matrix B indexing by transpose
	- Implement normal matrix multiply
	- Method 1 : implement matrix multiply with multiple row and col at the same time with blockSize
	- Method 2 : implement tiling multiply with blockSize x blockSize but slower than the method 1 
- threadMultiply folder = multithreading blocked matrix mulyiplication
	- Implement parallel load matrix using 2 threads
	- Implement parallel blocked (tiling) matrix multiplication using 4 threads
	- Method 1 : using Method 1 from single threading to parallelize, but slow over
	- Method 2: using method 2 from single threading and use optimal blocksize and faster by 25%
