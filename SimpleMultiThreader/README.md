#  Multithreading with Pthreads in C++: Parallel For Loops (1D and 2D)
### Divyansh Kumar Gautam (2023208) | Ishaan Raj (2023248)

## How to run
compile your files using the given Makefile
```
make
```
now run your compiled executables, for example
```
./matrix
./vector
```

## Features
1D Parallel For Loop: Splits a range into multiple chunks and executes each chunk in parallel using Pthreads.
2D Parallel For Loop: Divides a 2D grid into chunks and executes them in parallel, again using Pthreads.

1. Parallel 1D Loop
The parallel_for function for 1D splits the specified range [low, high) into multiple chunks based on the number of threads. Each chunk is processed by a separate thread.

Steps
- Calculate the chunk size: Divide the range [low, high) by the number of threads (numThreads).
- Create threads: Each thread is assigned a portion of the range (from low + i * chunk_size to low + (i + 1) * chunk_size).
- Lambda Execution: Each thread executes the provided lambda function for the indices in its assigned range.
- Join Threads: After all threads have been created, the pthread_join function waits for all threads to complete.
- Execution Time: The total time taken for the parallel execution is measured and printed.

2. Parallel 2D Loop
The parallel_for function for 2D splits the 2D grid defined by the ranges [low1, high1) and [low2, high2) into chunks. Each chunk is flattened and assigned to a thread for parallel processing.

Steps:
- Flatten the 2D range: Convert the 2D grid into a 1D array by calculating the total number of elements (range1 * range2).
- Create threads: Similar to the 1D case, divide the flattened range into chunks and assign them to threads.
- Lambda Execution: Each thread processes its chunk, converting the flattened index back into 2D coordinates (i, j).
- Join Threads: pthread_join is used to ensure that the main thread waits for all threads to finish.
- Execution Time: The total time taken for the parallel execution is measured and displayed.

in this program, locks are used in order to avoid race condition.
##Contribution
Both of us mutually contributed equally in the making of this assignment.
