# Matrix Computation Optimization and Benchmarking

Academic project by Bianca-Anastasia Mircea, May 2026.

Three C implementations of matrix/vector computations: baseline, BLAS, and manual optimization. The optimized implementation uses 40-element cache blocks, loop unrolling and matrix symmetry. Profiling artifacts include Valgrind Cachegrind/Memcheck logs and comparison plots.

## Evidence

The archived baseline Cachegrind log reports a 6.7% L1 data-cache miss rate; the optimized log reports 0.2%. These are results for the supplied workload, not a guarantee for every workload or platform. See `cache/neopt.cache` and `cache/opt_m.cache`.

## Files

- `src/`: the three solver implementations.
- `cache/` and `memory/`: archived profiling results.
- `grafice/`: analysis plots.
- `docs/original_README.md`: original submission documentation.
- `LLMprompts.md`: retained AI assistance disclosure.

## Reproducibility and limitations

The submission archive does not contain the original course driver, `utils.h`, Makefile or input datasets. It must be combined with the matching course skeleton to compile. A BLAS implementation is required for the BLAS solver. The current optimized loops assume matrix dimensions divisible by the 40-element block size. Do not treat this as a general-purpose matrix library. Archived benchmark results have not been rerun during portfolio preparation.
