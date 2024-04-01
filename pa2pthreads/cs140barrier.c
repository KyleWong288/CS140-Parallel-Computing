/*
 * File: cd140barrier.c
 *
 */

#include "cs140barrier.h"
#include <stdio.h>

/******************************************************
 * Initialize variables in a cs140barrier.
 * It should initialize the associated pthread mutex and condition variables
 * It should also set the initial values for other fields.
 * The odd_round field should be intiaized to be False.
 *
 * Argument:  bstate -- keep the state of a cs140barrier,
 *                      where bstate->odd_round is initialized to be False.
 *            total_nthread -- the number of threads using this barrier.
 *
 * Return:   0 successful, otherwise -1 meaning failed.
 */

int cs140barrier_init(cs140barrier *bstate, int total_nthread) {
  /*Your solution*/

  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_t cond;
  pthread_cond_init(&cond, NULL);

  bstate->barrier_mutex = mutex;
  bstate->barrier_cond = cond;
  bstate->total_nthread = total_nthread;
  bstate->arrive_nthread = 0;
  bstate->odd_round = False;

  return 0;
}

/******************************************************
 * This function shall synchronize participating threads at the barrier
 * referenced by bstate. The calling thread shall block
 * until the required number of threads have called cs140barrier_wait().
 * This completes this round of synchronization.
 *
 * Argument:  bstate -- keep the state of a cs140barrier.
 *
 * Return:   Upon successful completion, the last thread which observes all
 *           threads have arrived in this barrier round should return 1,
 *           and other threads should return 0.
 *
 * Algorithm: Use the sense reversal technique discussed in class.
 *            If a thread that calls this function is not the last thread,
 *            it should conditionally wait until the round flag changes.
 *            The last thread should change the flag and other fields.
 */


int cs140barrier_wait(cs140barrier *bstate) {
  /*Your solution*/

  int res = 0;
  boolean cur_round;

  pthread_mutex_lock(&(bstate->barrier_mutex));
  cur_round = bstate->odd_round;
  bstate->arrive_nthread += 1;
  if (bstate->arrive_nthread == bstate->total_nthread) {
    res = 1;
    bstate->odd_round ^= 1;
    bstate->arrive_nthread = 0;
    pthread_cond_broadcast(&(bstate->barrier_cond));
  }
  while (bstate->odd_round == cur_round) {
    pthread_cond_wait(&(bstate->barrier_cond), &(bstate->barrier_mutex));
  }
  pthread_mutex_unlock(&(bstate->barrier_mutex));
  
  return res;
}

/******************************************************
 * Destroy mutex and cond variables in a cs140barrier.
 * Note that the memory of bstate is not freed here.
 *
 * Argument:  bstate -- keep the state of a cs140barrier,
 *
 * Return:   0 successful, otherwise an error number is returned.
 */

int cs140barrier_destroy(cs140barrier *bstate) {
  /*Your solution*/
  
  pthread_mutex_destroy(&(bstate->barrier_mutex));
  pthread_cond_destroy(&(bstate->barrier_cond));
  
  return 0;
}
