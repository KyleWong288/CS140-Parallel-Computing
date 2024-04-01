/*
 * File:     treesum_mpi.c
 *
 * Purpose:  Use tree-structured communication to find the global sum
 *           of a random collection of ints.  This version doesn't
 *           require that comm_sz be a power of 2.
 *
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/*-------------------------------------------------------------------
 * Function:
 *  global_sum
 *
 * Purpose:
 *  Implement a global sum using tree-structured communication
 *
 * Notes:
 *  1.  The return value for global sum is only valid on process 0
 */
int global_sum(int my_int /* in */, int my_rank /* in */, int comm_sz /* in */,
               MPI_Comm comm /* in */) {
  int my_sum = my_int;

  /* Your solution */
  
  if (my_rank % 2) {
    // SEND
    MPI_Send(&my_sum, 1, MPI_INT, my_rank-1, 0, comm);
  }
  for (int gap=1; gap<comm_sz; gap*=2) {
    if (my_rank % (2 * gap) == 0) {
      int rec_sum = 0;
      if (my_rank + gap < comm_sz) {
        // RECEIVE
        MPI_Recv(&rec_sum, 1, MPI_INT, my_rank+gap, 0, comm, MPI_STATUS_IGNORE);
      }
      my_sum += rec_sum;
      if (my_rank % (4 * gap) && my_rank >= 2 * gap) {
        // SEND
        MPI_Send(&my_sum, 1, MPI_INT, my_rank-(2*gap), 0, comm);
      }
    }
  }

  return my_sum;
} /* Global_sum */
