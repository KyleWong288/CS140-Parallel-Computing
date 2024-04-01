/* File:     itmv_mult_pth.c
 *
 * Purpose:  Implement (the Jacobi methods) iteartive parallel matrix-vector
 *           multiplication with pthreads. Use one-dimensional arrays to store
 *           the vectors and the matrix.
 *
 * Algorithm:
 *        For k = 0 to t-1
 *            y = d + Ax
 *
 *            if abs(x_i, y_i) < threshold for i = 0 to n-1: break
 *
 *            x = y
 *        Endfor
 */
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "itmv_mult_pth.h"


pthread_barrier_t mybarrier; /*It will be initailized at itmv_mult_test_pth.c*/

/*---------------------------------------------------------------------
 * Function:            mv_compute
 * Purpose:             Compute  y[i]=d[i]+A[i]x for i-th element of vector y
 * In arg:              i -- row index
 * Global in vars:
 *        double matrix_A[]: 2D matrix A represented by a 1D array.
 *        double vector_d[]: vector d
 *        double vector_x[]: vector x
 *        matrix_type: matrix_type=0 means A is a regular matrix.
 *                     matrix_type=1 (UPPER_TRIANGULAR) means A is an upper 
 *                     triangular matrix
 *        matrix_dim: the global  number of columns (same as the number of rows)
 * Global in/out vars:
 *        double vector_y[]: vector y
 */
void mv_compute(int i) {
  int j, col_start, k;
  double tmp_y = vector_d[i];
  if (matrix_type == UPPER_TRIANGULAR) {
    col_start = i;
  } else {
    col_start = 0;
  }
  for (j = col_start; j < matrix_dim; j++) {
    tmp_y += matrix_A[i * matrix_dim + j] * vector_x[j];
  }
  vector_y[i] = tmp_y;
}

/*-------------------------------------------------------------------
 * Function:  itmv_mult_seq
 * Purpose:   Run t iterations of the Jacobi method (y=d+Ax) sequentially.
 * In args:   A:  matrix A
 *            d:  column vector d
 *            matrix_type:  matrix_type=0 means A is a regular matrix.
 *                          matrix_type=1 (UPPER_TRIANGULAR) means
 *                          A is an upper triangular matrix
 *            n: the global number of columns (same as the number of rows)
 *            t: the number of iterations
 * In/out:    x: column vector x 
 *            y: column vector y
 * Return:  1  means succesful 0 means unsuccessful
 *
 * Errors: If an error is detected (e.g. n is non-positive),
 *         matrix/vector pointers are NULL.
 */
int itmv_mult_seq(double A[], double x[], double d[], double y[],
                  int matrix_type, int n, int t) {
  int i, j, start, k, stop;

  if (n <= 0 || A == NULL || x == NULL || d == NULL || y == NULL) return 0;

  for (k = 0; k < t; k++) {
    // Do matrix-vector computation.
    for (i = 0; i < n; i++) {
      y[i] = d[i];
      if (matrix_type == UPPER_TRIANGULAR) {
        start = i;
      } else {
        start = 0;
      }
      for (j = start; j < n; j++) {
        y[i] += A[i * n + j] * x[j];
      }
    }
    
    // Check if reach convergence.
    stop = 1;
    for (i = 0; i < n && stop; i++) 
      if (fabs(x[i] - y[i]) > ERROR_THRESHOLD) {
        stop = 0;
      }

    if (stop) {
#ifdef DEBUG1
      printf("Reach convergence with %d iterations.\n", k);
#endif
      break;
    }

    // Update x.
    for (i = 0; i < n; i++) {
      x[i] = y[i];
    }
  }
  return 1;
}


/*---------------------------------------------------------------------
 * Function:  work_block
 * Purpose:   Run t iterations of parallel computation:  {y=d+Ax; x=y}
 *            based on block mapping. blocksize=ceil(matrix_dim/thread_count);
 *            For example, given 2 threads,
 *            Thread 0 should handle computation for Rows 0 and 1, and
 *            Thread 1 should handle computation for Rows 2 and 3.
 * In arg:    
 *            my_rank: rank of this thread (counted from 0)
 * Global in vars:
 *            thread_count
 *            double matrix_A[]:  2D matrix A represented by a 1D array.
 *            double vector_d[]: vector d
 *            int matrix_type:  matrix_type=0 means A is a regular matrix.
 *                              matrix_type=1 (UPPER_TRIANGULAR) means
 *                              A is an upper triangular matrix.
 *            int matrix_dim:  the global  number of columns
 *                             (same as the number of rows)
 *            int no_iteration:   the number of iterations
 * Global in/out vars:
 *            double vector_x[]:  vector x
 * Global out vars:
 *            double vector_y[]:  vector y
 */

void work_block(long my_rank) {

  int k = 0;
  double err = 1.0;

  // Find the rows this thread is responsible for:
  int block_size = (matrix_dim + thread_count - 1) / thread_count;
  int row_begin = block_size * my_rank;
  int row_end = fmin(block_size * (my_rank + 1), matrix_dim) - 1;
  
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
      if (err <= ERROR_THRESHOLD) {
        // printf("Thread %d converged with error %f on iteration %d\n", my_rank, err, k);
      }
    }
    pthread_barrier_wait(&mybarrier);

    // Update x for the next iteration:
    if (update) {
      for (int i=row_begin; i<=row_end; ++i) {
        vector_x[i] = vector_y[i];
      }
    }
    pthread_barrier_wait(&mybarrier);

    ++k;
  }
}



/*---------------------------------------------------------------------
 * Function:  work_blockcyclic
 * Purpose:   Run t iterations of parallel computation:  {y=d+Ax; x=y} 
 *            based on block cylic mapping
 *
 * In arg:
 *            my_rank:         rank of this thread (counted from 0)
 * Global in vars:
 *            int cyclic_blocksize:  basic block size used for
 *                                   block cylic mapping
 *            int thread_count: no of threads allocated
 *            double matrix_A[]:  2D matrix A represented by a 1D array.
 *            double vector_d[]:  vector d
 *            int matrix_type:  matrix_type=0 means A is a regular matrix.
 *                              matrix_type=1 (UPPER_TRIANGULAR) means
 *                              A is an upper triangular matrix
 *            int matrix_dim:  the global number of columns
 *                             (same as the number of rows)
 *            int no_iteration:   the number of iterations
 * Global in/out vars:
 *            double vector_x[]:  vector x
 * Global out vars:
 *            double vector_y[]:  vector y
 */

void work_blockcyclic(long my_rank) {
  /*Your solution*/

  int k = 0;
  double err = 1.0;
  int row_begin, row_end;
  
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
    pthread_barrier_wait(&mybarrier);
    if (err <= ERROR_THRESHOLD) {
      // printf("Thread %d converged with error %f on iteration %d\n", my_rank, err, k);
    }

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
    pthread_barrier_wait(&mybarrier);

    ++k;
  }

}