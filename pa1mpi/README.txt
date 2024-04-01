Last name of Student 1: Wong
First name of Student 1: Kyle
Email of Student 1: knw@ucsb.edu
Last name of Student 2:
First name of Student 2:
Email of Student 2:


Report for Question 2.a
Used CSIL with load averages of about 2

Parallel Time from NPROC=2:
    Test 11: 29.477006 s
    Test 12: 29.863057 s

Parallel Time from NPROC=4:
    Test 11: 15.597988 s
    Test 12: 15.314967 s

Speedup:
    Test 11: 29.477006 / 15.597988 = 1.889
    Test 12: 29.863057 / 15.314967 = 1.949
    
Efficiency:
    Test 11: (2 * 29.477006) / (4 * 15.597988) = 0.945
    Test 12: (2 * 29.863057) / (4 * 15.314967) = 0.975


Report for Question 2.b
Used CSIL with load averages of about 2

Parallel Time from NPROC=2:
    Test 11: 29.377844 s
    Test 12: 22.282824 s

Parallel Time from NPROC=4:
    Test 11: 15.085620 s
    Test 12: 13.200584 s

Speedup:
    Test 11: 29.377844 / 15.085620 = 1.947
    Test 12: 22.282824 / 13.200584 = 1.688
    
Efficiency:
    Test 11: (2 * 29.377844) / (4 * 15.085620) = 0.974
    Test 12: (2 * 22.282824) / (4 * 13.200584) = 0.844


The efficiency for Test 12 in 2.b is lower than in 2.a. 
This is because the load balancing is uneven for the upper triangular matrix.
In 2.a, each processor loops through all columns from 0 to n-1.
In 2.b, each processor loops from the diagonal starting point to n-1.
I use block mapping to partition my matrix A, so the first proc gets multiple mostly full rows, while the last proc gets multiple mostly empty rows.
So for 2.a, the efficiency is normal (near 1) since each proc is looping over the same amount of indices.
But for 2.b, our efficiency is poor since the start and end procs loop over a drastically different amount of indices.

The solution to this is to use a different data partitioning method than naive block partitioning.
We discussed cyclic partitioning in class, but there is a better way to partition in this case.
The best way to data partition matrix A is for each proc to view corresponding upper and lower rows.
This is because the amount of non-zero items in each row looks like [n-1, n-2, n-3, ... 3, 2, 1, 0].

Example:
n = 16, N_PROC = 4
Proc 0 views rows [0,1,14,15]. Total items = [n-1, n-2, 1, 0] = 2(n-1)
Proc 1 views rows [2,3,12,13].  Total items = [n-3, n-4, 3, 2] = 2(n-1)
Proc 2 views rows [4,5,10,11]. Total items = [n-5, n-6, 5, 4] = 2(n-1)
Proc 3 views rows [6,7,8,9].   Total items = [n-7, n-8, 7, 6] = 2(n-1)
In general, pairing the ith row and the (n-i-1)th row results in even load balancing.

This works because the ith row has n-i-1 items. 
So the (n-i-1)th row has (n - (n-i-1) - 1) items, which equals i items.
So then (ith row) + (n-ith row) = (n-i-1) + (i) = n - 1 items.
Thus, we can continually pair the ith row and the (n-i-1)th row to ensure even load balancing. 