
/*
 * File:     treesum_test_mpi.c
 *
 * Purpose:  test tree-structured communication to find the global sum
 *           of a random collection of ints.  This version doesn't
 *           require that no of processes  be a power of 2.
 *
 * Compile:  mpicc -Wall -o treesum_test_mpi  treesum_test_mpi.c treesum_mpi.c
 * minunit.c Run:      ibrun  -v ./treesum_test_mpi
 *
 * IPP:      Programming Assignment Text Book Problem 3.3 Page 148
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "minunit.h"

int my_rank, no_proc;
MPI_Comm comm;

int global_sum(int my_int, int my_rank, int no_proc, MPI_Comm comm);

/*-------------------------------------------------------------------
 * Test global_sum()
 * If successful, return NULL
 */
char *treesum_test1() {
  /* Your solution */
  double startwtime = 0, endwtime = 0;
  int expected = (no_proc + 1) * (no_proc) / 2;
  int arr[no_proc];
  for (int i=0; i<no_proc; ++i) {
    arr[i] = i+1;
  }

  if (my_rank == 0) startwtime = MPI_Wtime();
  int res = global_sum(arr[my_rank], my_rank, no_proc, comm);
  if (my_rank == 0) {
    endwtime = MPI_Wtime();
    double time = endwtime - startwtime;
    printf("Wall clock time = %f at Proc 0 of %d processes\n", time, no_proc);
    mu_assert("TreeSum Test 1: Wrong Sum", res == expected);
    printf("SUM: %d\n", res);
  }
  
  return NULL;
}

/*-------------------------------------------------------------------
 * Run all tests.  Ignore returned messages.
 */
void run_all_tests(void) { mu_run_test(treesum_test1); }

/*-------------------------------------------------------------------
 * The main entrance to run all tests.
 * Only Proc 0 prints the test summary
 */
int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &no_proc);
  MPI_Comm_rank(comm, &my_rank);

  run_all_tests();

  if (my_rank == 0) {
    mu_print_test_summary("Summary:");
  }
  MPI_Finalize();
  return 0;
}
