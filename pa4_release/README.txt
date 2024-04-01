Last name of Student 1: Wong
First name of Student 1: Kyle
Email of Student 1: knw@ucsb.edu
GradeScope account name of Student 1: Kyle Wong
Last name of Student 2:
First name of Student 2:
Email of Student 2:
GradeScope account name of Student 2: 


----------------------------------------------------------------------------
Report for Question 1 

List your code change for this question 

In the host function it_mult_vec(), I added the following code to transfer i/o between host and device, as well as call the Jacobi method:

  // copy A from host to device
  result = cudaMalloc( (void **) &A_d, A_size);
  if (result) {
    printf("Error in cudaMalloc for A. Error code is %d.\n", result);
    return -1;
  }
  result = cudaMemcpy(A_d, A, A_size, cudaMemcpyHostToDevice);
  if (result) {
    printf("Error in cudaMemcpy. Error code is %d.\n", result);
    return -1;
  }

  // call Jacobi function
  mult_vec<<<num_blocks, threads_per_block>>>(
        N, rows_per_thread, y_d, d_d, A_d, x_d, diff_d);
      k++;

  // copy y from device to host
  result = cudaMemcpy(y, y_d, row_size, cudaMemcpyDeviceToHost);
  if (result) {
    printf("Error in cudaMemcpy from device to host. Error code is %d.\n", result);
    return -1;
  }

In the kernel function mult_vec(), I added the following code to compute the starting index:

  // compute starting index
  idx = blockIdx.x * blockDim.x + threadIdx.x;


Parallel time for n=4K, t=1K,  4x128  threads: 3.961570 s

Parallel time for n=4K, t=1K,  8x128  threads: 1.983343 s

Parallel time for n=4K, t=1K,  16x128 threads: 1.000495 s

Parallel time for n=4K, t=1K,  32x128 threads: 0.534509 s


Do you see a trend of  speedup improvement  with more threads? We expect a good speedup and explain the reason.

ANSWER:
Speedup from 4x128 to 8x128: 3.961570 / 1.983343 = 1.997
Speedup from 4x128 to 16x128: 3.961570 / 1.000495 = 3.959
Speedup from 4x128 to 32x128: 3.961570 / 0.534509 = 7.411
The speedups are pretty close to linear speedup, especially for 8x128 and 16x128 (respectively, their speedup is close to the perfect speedup of 2x and 4x).
The speedup for 32x128 is a bit shy of the ideal 8x. This means we are starting to suffer from excessive overhead due to the high amount of threads used.
These good speedups are expected because we are improving parallelism by using more blocks/threads, so each thread has less data to individually compute. 
Similar to previous PAs like PA1 and PA2, we could improve performance by using more cores or more threads. So increasing the # of blocks used for CUDA has a similar effect.

----------------------------------------------------------------------------


Report for Question 2 
List your code change for this question

In the kernel function mult_vec_async(), I added the following code to compute the starting index and implement Gauss-Seidel:

  // compute starting index 
  idx = blockIdx.x * blockDim.x + threadIdx.x;

  // compute Gauss-Seidel
  for (int i = 0; i < rows_per_thread; ++i) {
    int row_index = idx * rows_per_thread + i;
    double sum = d[row_index];
    for (int j = 0; j < n; ++j) {
      sum += A[row_index * n + j] * y[j];
    }
    y[row_index] = sum;
  }



Let the default number of asynchronous iterations be 5 in a batch as specified in it_mult_vec.h.

List reported parallel time and the number of actual iterations executed  for n=4K, t=1K, 8x128  threads with asynchronous Gauss Seidel
  Parallel time: 0.040964
  Number of actual iterations: 15

List reported parallel time and the number of actual iterations executed  for n=4K, t=1K,  32x128 threads with asynchronous Gauss Seidel
  Parallel time: 0.059007
  Number of actual iterations: 100

Is the number of iterations  executed by  above parallel asynchronous Gauss Seidel-Seidel method  bigger or smaller  than that
of the sequential Gauss Seidel-Seidel code under the same converging error threshold (1e-3)?  
Explain the reason based on the running trace of above two thread configurations that more threads may not yield more time reduction in this case. 

ANSWER:
The number of iterations of sequential GS (tested by using num_blocks=1, threads_per_block=1) was 10 iterations, which was less than parallel asynchronous GS.
In theory, the more sequential our GS is (aka fewer blocks and threads), the fewer number of iterations it will take to converge.
Sequential GS converges fastest since it always uses the most up to date values for y.
However, parallel asynchronous GS doesn't wait for the entire y vector from the previous iteration to be updated.
This means the y vector it reads from may not have the most up to date values, slowing down convergence.
Futhermore, we see that less parallelism can perform better, as 8x128 outperformed 16x128.
More threads may be worse because higher parallelism means more chunks of y are unsynchronized. This creates more noise on what the "true" y vector should be, slowing down convergence.


----------------------------------------------------------------------------
Attach the  output trace  of your code below in running the tests of the unmodified it_mult_vec_test.cu on Expanse GPU

>>>>>>>>>>>>>>>>>>>>>>>>>
Start running itmv tests.
>>>>>>>>>>>>>>>>>>>>>>>>>

Test 1:n=4, t=1, 1x2 threads:
With totally 1*2 threads, matrix size being 4, t being 1
Time cost in seconds: 0.281109
Final error (|y-x|): 1.750000.
# of iterations executed: 1.
Final y[0]=1.750000. y[n-1]=1.750000

Test 2:n=4, t=2, 1x2 threads:
With totally 1*2 threads, matrix size being 4, t being 2
Time cost in seconds: 0.000246
Final error (|y-x|): 1.312500.
# of iterations executed: 2.
Final y[0]=0.437500. y[n-1]=0.437500

Test 3:n=8, t=1, 1x2 threads:
With totally 1*2 threads, matrix size being 8, t being 1
Time cost in seconds: 0.000230
Final error (|y-x|): 1.875000.
# of iterations executed: 1.
Final y[0]=1.875000. y[n-1]=1.875000

Test 4:n=8, t=2, 1x2 threads:
With totally 1*2 threads, matrix size being 8, t being 2
Time cost in seconds: 0.000226
Final error (|y-x|): 1.640625.
# of iterations executed: 2.
Final y[0]=0.234375. y[n-1]=0.234375

Test 8a:n=4, t=1, 1x1 threads/Gauss-Seidel:
With totally 1*1 threads, matrix size being 4, t being 1
Time cost in seconds: 0.000220
Final error (|y-x|): 1.000193.
# of iterations executed: 5.
Final y[0]=1.000089. y[n-1]=1.000193

Test 8b:n=4, t=2, 1x1 threads/Gauss-Seidel:
With totally 1*1 threads, matrix size being 4, t being 2
Time cost in seconds: 0.000228
Final error (|y-x|): 1.000193.
# of iterations executed: 5.
Final y[0]=1.000089. y[n-1]=1.000193

Test 8c:n=8, t=1, 1x1 threads/Gauss-Seidel:
With totally 1*1 threads, matrix size being 8, t being 1
Time cost in seconds: 0.000234
Final error (|y-x|): 1.001155.
# of iterations executed: 5.
Final y[0]=1.001155. y[n-1]=0.999790

Test 8d:n=8, t=2, 1x1 threads/Gauss-Seidel:
With totally 1*1 threads, matrix size being 8, t being 2
Time cost in seconds: 0.000237
Final error (|y-x|): 1.001155.
# of iterations executed: 5.
Final y[0]=1.001155. y[n-1]=0.999790

Test 9: n=4K t=1K 32x128 threads:
With totally 32*128 threads, matrix size being 4096, t being 1024
Time cost in seconds: 0.534509
Final error (|y-x|): 1.557740.
# of iterations executed: 1024.
Final y[0]=0.221225. y[n-1]=0.221225

Test 9a: n=4K t=1K 16x128 threads:
With totally 16*128 threads, matrix size being 4096, t being 1024
Time cost in seconds: 1.000495
Final error (|y-x|): 1.557740.
# of iterations executed: 1024.
Final y[0]=0.221225. y[n-1]=0.221225

Test 9b: n=4K t=1K 8x128 threads:
With totally 8*128 threads, matrix size being 4096, t being 1024
Time cost in seconds: 1.983343
Final error (|y-x|): 1.557740.
# of iterations executed: 1024.
Final y[0]=0.221225. y[n-1]=0.221225

Test 9c: n=4K t=1K 4x128 threads:
With totally 4*128 threads, matrix size being 4096, t being 1024
Time cost in seconds: 3.961570
Final error (|y-x|): 1.557740.
# of iterations executed: 1024.
Final y[0]=0.221225. y[n-1]=0.221225

Test 11: n=4K t=1K 32x128 threads/Async:
With totally 32*128 threads, matrix size being 4096, t being 1024
Time cost in seconds: 0.059007
Final error (|y-x|): 0.000850.
# of iterations executed: 100.
Early exit due to convergence, even asked for 1024 iterations.
Asynchronous code actually runs 100 iterations.
Final y[0]=0.999666. y[n-1]=0.999660

Test 11a: n=4K t=1K 8x128 threads/Async:
With totally 8*128 threads, matrix size being 4096, t being 1024
Time cost in seconds: 0.040964
Final error (|y-x|): 0.000032.
# of iterations executed: 15.
Early exit due to convergence, even asked for 1024 iterations.
Asynchronous code actually runs 15 iterations.
Final y[0]=1.000000. y[n-1]=1.000000

Summary: Failed 0 out of 14 tests
