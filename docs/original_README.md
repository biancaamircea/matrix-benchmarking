# Computer Systems Architecture Assignment 1 -- Matrix Multiplication Optimization

## Overview

This project contains three implementations of a matrix/vector computation. It compares an unoptimized handwritten implementation, a BLAS implementation and a manually optimized implementation focused on memory access.

The implemented equations are:

```text
C = At * B
D = C * Ct
y = D * (row sums of C) + x
```

Here, `At` and `Ct` denote the transposes of A and C.

## Implementation details

### 1. Unoptimized implementation (`solver_neopt.c`)

This is the reference implementation. The first multiplication, C = At * B, uses three nested loops. Instead of physically transposing A, the code accesses A[k][i], or `A[k * N + i]` in linear storage.

For D = C * Ct, the implementation exploits the symmetry specified in the assignment. It computes the main diagonal and one triangle (`j <= i`), then copies each result to the symmetric position using `D[j * N + i] = intermediate_sum`. This roughly halves the computations required for D.

Next, the row sums of C are stored in a separate vector. D is multiplied by this vector, and x is added to the result.

### 2. BLAS implementation (`solver_blas.c`)

This version uses CBLAS functions:

- Memory is allocated using `calloc`.
- `cblas_dgemm` computes C = At * B. Setting `TransA` to `CblasTrans` avoids a separate transpose.
- `cblas_dsyrk` computes D = C * Ct, exploiting symmetry and writing only the upper triangle (`CblasUpper`).
- Repeated `cblas_daxpy` calls accumulate the rows of C into the intermediate vector v.
- `cblas_dcopy` copies x into y.
- `cblas_dsymv` multiplies symmetric matrix D by the intermediate vector and adds the result to y.

### 3. Manually optimized implementation (`solver_opt.c`)

This version improves the baseline through code-level changes:

- **Blocked matrix multiplication:** 40-by-40 blocks (`B_SIZE = 40`) improve spatial and temporal locality, allowing data to be reused while processing a block.
- **Loop unrolling:** inner-loop iterations are grouped in fours (`j += 4`), reducing loop branches and allowing more arithmetic instructions to be scheduled together.
- **Local pointers and the register keyword:** row-start addresses are precomputed, for example `c_row = &C[i * N]`, to avoid repeated index calculations. Frequently used variables are declared with `register` as a suggestion to the compiler; actual register allocation remains the compiler's decision.

## Performance analysis: Valgrind Cachegrind and Memcheck

Memcheck runs on `input_valgrind` reported zero errors and zero memory leaks for all three implementations. All allocated memory was released, with no invalid accesses or uninitialized reads reported.

### 1. Instruction references (`I refs`)

- The baseline executes approximately 3.55 billion instructions.
- The manually optimized version reduces this to approximately 1.40 billion. Unrolling and precomputed pointers reduce the required instructions to less than half.
- BLAS executes approximately 83 million instructions, illustrating the efficiency of the precompiled library's optimized implementation.

### 2. Data accesses and L1 data-cache misses

- The baseline has a D1 miss rate of 6.7%, approximately 132 million misses. Column-wise access leads to poor locality.
- The manually optimized version reduces the miss rate to approximately 0.2%, or 781 thousand misses. Blocking improves reuse of data in the cache.

### 3. Branches

Unrolling reduces the optimized version's branch count from approximately 97 million to 20 million. Although its branch misprediction rate increases to approximately 8%, the lower total branch and instruction counts accompany a much shorter execution time.

## Bonus: Haswell versus UCSX

The generated Cachegrind logs for the optimized version report:

- **Haswell:** 781,524 D1 misses.
- **UCSX:** 705,909 D1 misses.
- The reported miss rate decreases from approximately 0.18% to 0.14%.

The original analysis interprets these differences as a possible advantage in cache behavior on UCSX, potentially involving cache configuration or prefetching. The logs alone do not establish which hardware mechanism causes the difference. Instruction references remain approximately the same, at 1.4 billion.

## Plots and execution times

Tests cover matrix sizes from N = 400 to N = 1800. The original analysis discusses the following 13 plots. Plot filenames are preserved.

### 1. Overall performance and scalability

- **timpi_neopt_blas_opt.png:** at N = 1200, the baseline takes approximately 13.6 s, the manually optimized version approximately 3.5 s, and BLAS approximately 0.17 s.
- **scalare_opt_m.png:** the optimized version exhibits the expected O(N^3) scaling, increasing from approximately 0.15 s to 11.6 s for the largest matrix.

### 2. Memory and instruction analysis

- **acces_memorie_drefs.png:** at N = 400, the baseline performs almost 2 billion memory references. Local pointers and other optimizations reduce this to approximately 471 million.
- **misses_l1_l3_variante.png:** L1 misses decrease from approximately 132 million in the baseline to fewer than 1 million in the optimized version.
- **rate_miss_si_branch.png:** L1 miss rate drops from 6.7% to 0.2%, while branch misprediction rate increases from 0.3% to approximately 8%. Overall execution time still improves.

### 3. Bonus: execution times on Haswell and UCSX

Tests were also run on the UCSX queue to compare the systems.

- **arhitecturi_timpi_global.png:** the line plot shows lower execution times on UCSX for all implementations and tested matrix sizes.
- **arhitecturi_grupate_n.png:** at N = 1800, the baseline decreases from approximately 50 s on Haswell to approximately 37 s on UCSX.
- **optimizat_haswell_vs_ucsx.png:** the optimized version takes approximately 8.6 s on UCSX versus 11.9 s on Haswell at N = 1800.
- **speedup_ucsx_opt_m.png:** from N = 1200 onward, the optimized version's UCSX speedup is approximately 1.35x to 1.39x.

### 4. Bonus: cache and branch comparisons

- **drefs_haswell_ucsx.png:** total memory-reference counts are identical across systems for the same code.
- **misses_arhitecturi_n400.png:** UCSX reports fewer misses overall in L1 and L3.
- **d1_miss_comparatie_noduri.png:** the optimized version has approximately 705 thousand L1 misses on UCSX versus 781 thousand on Haswell. The original analysis suggests prefetching as a possible explanation, rather than a measured hardware property.
- **rate_comparatie_arhitecturi.png:** branch misprediction is approximately 8% for the optimized version on both systems. For the baseline, the reported cache miss rate decreases from 6.7% on Haswell to 5.4% on UCSX.
