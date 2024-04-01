Last name of Student 1: Wong
First name of Student 1: Kyle
Email of Student 1: knw@ucsb.edu
Last name of Student 2:
First name of Student 2:
Email of Student 2:


If CSIL is used for performance assessment instead of Expanse, make sure you evaluate when such a machine is lightly 
loaded using “uptime”. Please indicate your evaluation is done on CSIL and list the uptime index of that CSIL machine.  

Report 
----------------------------------------------------------------------------
1. How is the code parallelized? Show your solution by listing the key computation parallelized with
  OpenMP and related code. 


----------------------------------------------------------------------------
2. Parallel time/speedup/efficiency using 2 and 4 cores under different scheduling/mapping methods
    for a full matrix (n=4096 and t=1024) on Expanse.

I used CSIL for measuring performance instead of Expanse. The uptime load averages were about 1.5.

Results for sequential:

  Test 9 runtime: 58.200 s
  Test 10 runtime: 58.146 s
  Test 11 runtime: 58.479 s
  Test 11a runtime: 57.899 s

Results for 2 threads:

  Test 9 runtime: 29.364 s
  Test 10 runtime: 31.828 s
  Test 11 runtime: 29.508 s
  Test 11a runtime: 29.690 s

  Test 9 speedup: 58.200 / 29.364 = 1.982
  Test 10 speedup: 58.146 / 31.828 = 1.827
  Test 11 speedup: 58.479 / 29.508 = 1.982
  Test 11a speedup: 57.899 / 29.690 = 1.950

  Test 9 efficiency: 1.982 / 2 = 0.991
  Test 10 efficiency: 1.827 / 2 = 0.914
  Test 11 efficiency: 1.982 / 2 = 0.991
  Test 11a efficiency: 1.950 / 2 = 0.975

Results for 4 threads:

  Test 9 runtime: 14.866 s
  Test 10 runtime: 16.172 s
  Test 11 runtime: 14.805 s
  Test 11a runtime: 15.016 s

  Test 9 speedup: 58.200 / 14.866 = 3.915
  Test 10 speedup: 58.146 / 16.172 = 3.595
  Test 11 speedup: 58.479 / 14.805 = 3.950
  Test 11a speedup: 57.899 / 15.016 = 3.856

  Test 9 efficiency: 3.915 / 4 = 0.979
  Test 10 efficiency: 3.595 / 4 = 0.899
  Test 11 efficiency: 3.950 / 4 = 0.988
  Test 11a efficiency: 3.856 / 4 = 0.964


----------------------------------------------------------------------------
3. Parallel time/speedup/efficiency using 2 and 4 cores under different scheduling/mapping methods
 for an upper triangular matrix (n=4096 and t=1024) on Expanse.
Explain the above significant performance differences of the above methods with a short reason. 

I used CSIL for measuring performance instead of Expanse. The uptime load average were about 2.4.

Results for sequential:

  Test 12 runtime: 29.884 s
  Test 13 runtime: 29.976 s
  Test 14 runtime: 29.977 s
  Test 14a runtime: 30.324 s

Results for 2 threads:

  Test 12 runtime: 17.096 s
  Test 13 runtime: 16.324 s
  Test 14 runtime: 15.427 s
  Test 14a runtime: 15.184 s

  Test 12 speedup: 29.884 / 17.096 = 1.748
  Test 13 speedup: 29.976 / 16.324 = 1.836
  Test 14 speedup: 29.977 / 15.427 = 1.943
  Test 14a speedup: 30.324 / 15.184 = 1.997

  Test 12 efficiency: 1.748 / 2 = 0.874
  Test 13 efficiency: 1.836 / 2 = 0.918
  Test 14 efficiency: 1.943 / 2 = 0.972
  Test 14a efficiency: 1.997 / 2 = 0.999

Results for 4 threads:

  Test 12 runtime: 9.140 s
  Test 13 runtime: 8.608 s
  Test 14 runtime: 7.973 s
  Test 14a runtime: 7.688 s

  Test 12 speedup: 29.884 / 9.140 = 3.270
  Test 13 speedup: 29.976 / 8.608 = 3.482
  Test 14 speedup: 29.977 / 7.974 = 3.759
  Test 14a speedup: 30.324 / 7.688 = 3.944

  Test 12 efficiency: 3.270 / 4 = 0.818
  Test 13 efficiency: 3.482 / 4 = 0.8705
  Test 14 efficiency: 3.759 / 4 = 0.940
  Test 14a efficiency: 3.944 / 4 = 0.986

Explanation:

In general, the runtimes measured in (3.) were about half of the runtimes measured in (2.). 
This is because the triangular matrices have about half of its entries skipped due to being 0.

In general, the speedup and efficiencies in (2.) were better than in (3.). 
This is because the dense matrix in (2.) will always have an extremely even load balancing, as every row is filled.
Meanwhile, the upper triangular matrix in (3.) is harder to evenly load balance, since data is not distributed evenly and is instead skewed towards rows closer to 0.
This is why block mapping (Test 12) has the lowest efficiency, while pure/block cyclic mapping (Tests 13, 14) have higher efficiencies. 
Block mapping will give the first thread multiple mostly full rows, while giving the last thread multiple mostly empty rows, creating an uneven load balance.
Meanwhile, cyclic mapping attempts to group some mostly full rows with some mostly empty rows to each thread, creating a more even load balance. 
Lastly, dynamic mapping gets slightly better efficiency, due to dynamically managing threads so work is evenly distributed. 