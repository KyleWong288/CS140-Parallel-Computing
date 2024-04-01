#!/bin/bash  
# Next line shows the job name you can find when querying the job status
#SBATCH --job-name="itmv"
# Next line is the output file name of the execution log
#SBATCH --output="job_itmv_4nodes.%j.out"
# Next line shows where to ask for machine nodes
#SBATCH --partition=shared
#Next line asks for 1 node and  4 cores per node 
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4
#SBATCH --export=ALL
# Next line limits the job execution time at most 3 minute.
#SBATCH -t 00:03:00

#SBATCH --account=csb175
#csb175 is the class-wide account shared among all cs140 students


#ibrun in verbose mode will give binding detail. It uses one MPI process per core.


module purge
module load slurm
module load cpu
module load gcc
module load openmpi
module load sdsc

ibrun -v ./itmv_mult_test_mpi
