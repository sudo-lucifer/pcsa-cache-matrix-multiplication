#!/usr/bin/python3

import numpy as np
import os,sys
import time

size = sys.argv[1]

f1 = open("input1.in","w")
f2 = open("input2.in","w")
f3 = open("reference.in","w")

matrix1 = np.array([np.random.randint(6, size=int(size)) for i in range(int(size))])
matrix2 = np.array([np.random.randint(6, size=int(size)) for i in range(int(size))])
	
start = time.time()
for i in range(int(size)):
	[f1.write(str(k) + " ") for k in matrix1[i]]
	[f2.write(str(k) + " ") for k in matrix2[i]]
	f1.write("\n")
	f2.write("\n")

result = np.matmul(matrix1,matrix2)
for i in result:
	[f3.write(str(j) + " ") for j in i]
	f3.write("\n")
		
end = time.time()
print("Elased Time: " + str(end - start))

f1.close()
f2.close()
f3.close()