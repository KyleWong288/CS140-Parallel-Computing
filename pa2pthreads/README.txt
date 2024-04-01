Last name of Student 1: Wong
First name of Student 1: Kyle
Email of Student 1: knw@ucsb.edu
Last name of Student 2:
First name of Student 2:
Email of Student 2:

Q1: List parallel code that uses at most two barrier calls inside the while loop

-----------------------------------------------------------------------------------------------------------------------------------

Parallel loop for block cyclic:

  while (k < no_iterations && err > ERROR_THRESHOLD) {

    // Compute y[i] and convergence error:
    err = 0;
    row_begin = my_rank * cyclic_blocksize;
    row_end = fmin(row_begin + cyclic_blocksize, matrix_dim) - 1;
    while (row_begin < matrix_dim) {
      for (int i=row_begin; i<=row_end; ++i) {
        mv_compute(i);
        err = fmax(err, fabs(vector_y[i] - vector_x[i]));
      }
      row_begin += (thread_count * cyclic_blocksize);
      row_end += (thread_count * cyclic_blocksize);
      row_end = fmin(row_end, matrix_dim - 1);
    }
    pthread_barrier_wait(&mybarrier);   // BARRIER 1

    // Update x for the next iteration:
    row_begin = my_rank * cyclic_blocksize;
    row_end = fmin(row_begin + cyclic_blocksize, matrix_dim) - 1;
    while (row_begin < matrix_dim) {
      for (int i=row_begin; i<=row_end; ++i) {
        vector_x[i] = vector_y[i];
      }
      row_begin += (thread_count * cyclic_blocksize);
      row_end += (thread_count * cyclic_blocksize);
      row_end = fmin(row_end, matrix_dim - 1);
    }
    pthread_barrier_wait(&mybarrier);   // BARRIER 2

    ++k;
  }

-----------------------------------------------------------------------------------------------------------------------------------

Parallel loop for block mapping:

  while (k < no_iterations) {
    int update = 0;

    // Compute y[i] and convergence error
    if (err > ERROR_THRESHOLD) {
      err = 0;
      update = 1;
      for (int i=row_begin; i<=row_end; ++i) {
        mv_compute(i);
        err = fmax(err, fabs(vector_y[i] - vector_x[i]));
      }
    }
    pthread_barrier_wait(&mybarrier);   // BARRIER 1

    // Update x for the next iteration:
    if (update) {
      for (int i=row_begin; i<=row_end; ++i) {
        vector_x[i] = vector_y[i];
      }
    }
    pthread_barrier_wait(&mybarrier);   // BARRIER 2

    ++k;
  }

-----------------------------------------------------------------------------------------------------------------------------------

Q2: Report parallel time, speedup, and efficiency for the upper triangular test matrix case when n=4096 and t=1024. 
Use 2 threads and 4 threads (1 thread per core) under blocking mapping, and block cyclic mapping with block size 1 and block size 16.    

I used CSIL for measuring performance instead of Expanse. 
Uptime results: 11:47:38 up 26 days, 35 min,  6 users,  load average: 2.93, 3.25, 3.64

Results for sequential:

    Test 12 runtime: 0.084210 s
    Test 13 runtime: 0.085405 s
    Test 14 runtime: 0.083866 s

Results for 2 threads:

    Test 12 runtime: 0.103933 s
    Test 13 runtime: 0.046497 s
    Test 14 runtime: 0.046693 s

    Test 12 speedup: 0.084210 / 0.103933 = 0.810
    Test 13 speedup: 0.085405 / 0.046497 = 1.836
    Test 14 speedup: 0.083866 / 0.046693 = 1.796

    Test 12 efficiency: 0.810 / 2 = 0.405
    Test 13 efficiency: 1.836 / 2 = 0.918
    Test 14 efficiency: 1.796 / 2 = 0.898

Results for 4 threads:

    Test 12 runtime: 0.088780 s
    Test 13 runtime: 0.025489 s
    Test 14 runtime: 0.026166 s

    Test 12 speedup: 0.084210 / 0.088780 = 0.948
    Test 13 speedup: 0.085405 / 0.025489 = 3.350
    Test 14 speedup: 0.083866 / 0.026166 = 3.205

    Test 12 efficiency: 0.948 / 4 = 0.237
    Test 13 efficiency: 3.350 / 4 = 0.837
    Test 14 efficiency: 3.205 / 4 = 0.801


Explanation:

In general, block mapping (Test 12) is extremely inefficient, while block cyclic mapping (Tests 13 and 14) have fairly expected results.
For 2 threads, the speedup and efficiency for Test 13 and 14 were close to the ideal values of 2.0 and 1.0.
For 4 threads, the speedup and efficiency for Test 13 and 14 were also close to the ideal values of 4.0 and 1.0.
However, for Test 12, the efficiencies for 2 and 4 threads were closer to 1/2 and 1/4, instead of 1.

This is because we are testing on the upper triangular matrix, so block mapping has the most uneven load balancing. 
The first thread gets many "mostly full rows", while the last thread gets many "mostly empty rows", creating an uneven load balance.
Thus, no matter how many threads we use, we are bottlenecked by the first thread, and our runtime ends up similar to the sequential result.
This explains why the efficiency for 2 threads is around 1/2, and the efficiency for 4 threads is around 1/4.

Meanwhile, block cyclic performs a lot better than block mapping.
This is because block cyclic attempts to group some "mostly full rows" with some "mostly empty rows" to each thread, creating a more even load balance.
Also observe that r=1 performs better than r=16, both with 2 threads and with 4 threads.  
This is because r=16 maps more "mostly full rows" to the same thread.
In general, a larger r will approach the original block mapping distribution, which will revert back to poor load balancing.
