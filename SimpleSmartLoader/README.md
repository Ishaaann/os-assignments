# SimpleSmartLoader: ELF Loader with Lazy Page Allocation
> Divyansh Kumar Gautam (2023208)

> Ishaan Raj (2023248)

### Overview
SimpleSmartLoader is a custom ELF (Executable and Linkable Format) loader written in C that demonstrates lazy page allocation with fault handling. Instead of loading all program segments upfront, this loader maps memory only when needed, handling segmentation faults dynamically to optimize memory usage.

### Features
Lazy Loading: Does not allocate memory for segments initially, including the segment containing the entry point.
On-Demand Page Allocation: Handles segmentation faults as page faults by dynamically allocating memory using mmap().
Page-by-Page Allocation: Allocates memory in 4KB pages, loading segments incrementally as they are accessed.
Seamless Execution: Resumes program execution after handling page faults without terminating the process.
Statistics Reporting: Tracks and reports the total number of page faults and memory allocations.

### Implementation Details
1) No Upfront Allocation:
Directly executes the _start method of the ELF file.
Initially generates a segmentation fault because memory pages are not pre-allocated.
2) Handling Segmentation Faults:
Treats segmentation faults as page faults when accessing unallocated but valid memory addresses.
Allocates only the required page (4KB) and loads data from the ELF file as needed.
3) Incremental Loading:
Loads segment data one page at a time.
For example, if a segment is 5KB, it allocates 4KB initially, and another 4KB page is allocated upon accessing the remaining data.
4) Efficient Memory Usage:
Ensures minimal memory allocation by only mapping necessary pages during execution.
Tracks internal fragmentation and optimizes page utilization.

## How to Use
```make```
```./loader <ELF executable>```

### Contribution
Both of us mutually contributed equally in the making of this assignment.  

[Github Repo Link](https://github.com/Ishaaann/os-assignments)
