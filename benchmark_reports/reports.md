# Performance Profiling and Baseline Analysis

## 1. Experimental Methodology

The parser was evaluated using a real Nasdaq TotalView-ITCH 5.0 binary market-data capture rather than a synthetic workload. The capture contains **8,696,559 complete ITCH messages** and is approximately 249 MiB in size.

The experimental workflow follows a measurement-first methodology:

1. Establish a functionally correct baseline implementation.
2. Measure end-to-end execution time over the complete capture.
3. Collect hardware performance counters using Linux `perf`.
4. Identify CPU hotspots using sampled profiling.
5. Form a specific performance hypothesis from the measurements.
6. Modify one aspect of the implementation at a time.
7. Re-run the same workload and compare against the baseline.

This separation is important because an observed hotspot is not automatically a valid optimization target. A change is considered an optimization only if it produces a reproducible improvement in end-to-end performance.

---

## 2. Baseline Throughput

The baseline parser was compiled with GCC using C++20 and `-O2`. Five ordinary benchmark executions were performed on the complete ITCH capture.

The measured execution times were:

| Run | Time (s) |
| --- | -------: |
| 1   | 0.233147 |
| 2   | 0.234459 |
| 3   | 0.236648 |
| 4   | 0.238157 |
| 5   | 0.238831 |

The mean execution time was:

**0.23625 s**

For 8,696,559 complete messages, this corresponds to approximately:

**36.81 million messages/s**

and approximately:

**1.05 GiB/s of input throughput**.

This measurement is treated as the **B0 baseline** against which subsequent source-level and compiler-level experiments will be compared.

---

## 3. Hardware Performance Counter Analysis

Linux `perf stat -r 5` was used to collect hardware performance counters over five repetitions of the same workload.

The measured averages were:

| Counter          |  Measurement |
| ---------------- | -----------: |
| CPU cycles       |  1.037 × 10⁹ |
| Instructions     |  2.031 × 10⁹ |
| Branches         | 369.21 × 10⁶ |
| Branch misses    |  4.878 × 10⁶ |
| Branch miss rate |        ~1.3% |
| Task-clock       |    439.29 ms |
| Elapsed time     |    440.53 ms |

The resulting instructions-per-cycle (IPC) is approximately:

$$
IPC = \frac{2.031\times10^9}{1.037\times10^9}
\approx 1.96
$$

Thus, the baseline executes approximately **1.96 instructions per CPU cycle** during the measured workload.

The branch miss rate is approximately **1.3%**, indicating that conventional branch misprediction is not immediately identified as the dominant source of execution cost.

The Topdown Level-1 classification reported:

| Category        | Percentage |
| --------------- | ---------: |
| Backend bound   |      17.5% |
| Bad speculation |      68.0% |
| Frontend bound  |       6.3% |
| Retiring        |       8.2% |

The large `bad_speculation` classification requires further investigation. It should **not** be interpreted as equivalent to a 68% branch-misprediction rate, since the directly measured branch-miss rate is only approximately 1.3%.

At this stage, the Topdown result is therefore treated as an observation rather than a conclusion about the source of speculation-related cost.

---

## 4. CPU Hotspot Analysis

`perf record` was used to obtain a sampled CPU profile of the baseline implementation.

The most significant entries were:

| Function                       | Sampled CPU share |
| ------------------------------ | ----------------: |
| `__memset_evex_unaligned_erms` |            16.26% |
| `_copy_to_iter`                |             8.33% |
| `BinaryReader::read_u16_be`    |             7.88% |
| `Parser::parse_add_order`      |             7.87% |
| `BinaryReader::read_u48_be`    |             6.71% |
| `main`                         |             6.26% |
| `Parser::parse_order_delete`   |             5.79% |
| `BinaryReader::read_u64_be`    |             5.56% |
| `BinaryReader::read_u32_be`    |             4.86% |
| `Parser::parse_message`        |             3.94% |

The kernel functions appearing in the profile are not currently treated as direct optimization targets because the profile alone does not establish the precise mechanism responsible for their execution.

### 4.1 Message-type correlation

The parser's hottest message-processing functions correspond closely to the actual workload distribution. The capture contains approximately:

* 3.70 million Add Order (`A`) messages
* 3.54 million Order Delete (`D`) messages

Consequently, `parse_add_order` and `parse_order_delete` appearing prominently in the CPU profile is consistent with the workload composition.

This provides a useful sanity check that the profile is reflecting the actual message distribution rather than an artificial benchmark artifact.

---

## 5. `memset` Investigation

The largest individual user-space hotspot is:

```text
__memset_evex_unaligned_erms    16.26%
```

The parser constructs temporary message structures containing fixed-size character arrays. Some of these arrays were explicitly zero-initialized using `{}` before being populated using `read_bytes()`.

A preliminary experiment removed these redundant zero-initializations. The sampled `memset` share decreased from approximately **16.3% to 13.3%**.

However, the corresponding five-run wall-clock benchmark did not improve; the mean execution time was approximately **0.2556 s** compared with approximately **0.2547 s** for the comparison baseline. The difference was within normal run-to-run variation.

Therefore, the experiment demonstrates that the initializers contribute to `memset` activity, but it does **not** demonstrate an end-to-end performance improvement.

The explicit zero-initializations were consequently restored in the B0 baseline.

This distinction is important:

> A reduction in the percentage of CPU samples attributed to a function does not necessarily imply an improvement in total application performance.

---

## 6. BinaryReader Hotspot

A particularly significant observation is the combined contribution of the big-endian integer decoding functions:

```text
read_u16_be    7.88%
read_u48_be    6.71%
read_u64_be    5.56%
read_u32_be    4.86%
```

Together these account for approximately **25% of sampled CPU time**.

To investigate this hotspot, the generated x86-64 machine code was inspected using `objdump`.

The disassembly shows that the BinaryReader functions remain as distinct callable functions in the optimized executable. For example, `parse_add_order` contains explicit calls to `read_u48_be`, `read_u64_be`, and `read_u32_be`.

The generated implementation of `read_u16_be` performs a bounds check followed by two byte loads, a shift, and an OR operation.

Similarly, `read_u32_be` performs a bounds check followed by four byte loads and the corresponding shift/OR operations required to reconstruct the big-endian integer.

The six-byte timestamp decoder, `read_u48_be`, performs six byte loads and combines them through shifts and OR operations. It also performs a bounds check before decoding.

The 64-bit decoder follows the same byte-by-byte reconstruction approach for eight bytes.

Therefore, the profiler hotspot is supported by the generated machine code: each decoded field involves not only the byte-order conversion itself, but also a function-call boundary and a bounds check.

---

## 7. First Performance Hypothesis

The current evidence motivates the following hypothesis:

### H1 — BinaryReader call and bounds-check overhead

The repeated function calls and per-field bounds checks introduced by the `BinaryReader` abstraction contribute significantly to ITCH parsing overhead because each message invokes multiple checked decoding operations.

This hypothesis is supported by three independent observations:

1. The endian-decoding functions collectively account for approximately 25% of sampled CPU time.
2. The generated executable contains explicit calls to these functions rather than eliminating the abstraction completely.
3. Each function performs its own bounds check before decoding the requested field.

However, this remains a **hypothesis**, not yet a demonstrated optimization.

---

## 8. Proposed Experiment H1-A: Link-Time Optimization

The first controlled test will change only the compilation model, without changing parser source code.

The baseline is currently compiled from multiple translation units:

```text
binary_reader.cpp
itch_parser.cpp
main.cpp
```

The proposed experiment enables GCC Link-Time Optimization using:

```text
-flto
```

The purpose is to determine whether the compiler can see across translation-unit boundaries and eliminate or reduce the cost of the `BinaryReader` function calls.

The experiment therefore tests:

> Does exposing the complete program to the optimizer reduce the runtime cost associated with the BinaryReader abstraction?

Only the compiler configuration will be changed. The workload, parser algorithm, data structures, and source code remain unchanged.

The result will be evaluated using:

1. Five ordinary end-to-end benchmark runs.
2. Comparison against B0 mean runtime.
3. Inspection of the generated assembly.
4. A new `perf record` profile if the change produces a meaningful performance difference.

No other optimization will be introduced simultaneously, allowing any observed change to be attributed to the LTO experiment with greater confidence.

---

## 9. Current Research Status

At this stage, the baseline profiling has established three important characteristics of the workload:

* A substantial fraction of CPU samples is associated with temporary-object initialization (`memset`).
* Big-endian field decoding through `BinaryReader` represents a major user-space hotspot.
* The parser's hottest message-specific functions correspond to the dominant message types in the real capture.

The next experiment therefore focuses on the BinaryReader path while preserving the existing parser design.

The guiding principle remains:

**Measure → hypothesize → change one variable → measure again → inspect the resulting machine behavior.**



