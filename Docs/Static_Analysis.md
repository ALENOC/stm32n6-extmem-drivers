# Static Analysis

Static analysis of the 17 driver sources (`Drivers/BSP/Components/**` and `Drivers/BSP/STM32N6_ExtMem/*.c`),
run on the host with the HAL mock headers (`-DEXTMEM_UNIT_TEST`). No hardware is involved.

```bash
make static-analysis   # needs gcc, cppcheck (with its MISRA addon) and clang-tidy
```

The same target runs in CI (`static-analysis` job, Ubuntu 24.04, cppcheck 2.13, clang-tidy 22.1.8).

## Tools and results

| Tool | Checks | Result | Gate |
|:---|:---|:---|:---|
| GCC 13 `-fanalyzer` | Interprocedural analysis: NULL dereference, use of uninitialised data, buffer overflow, leaks | 0 findings | Fails on any finding (`-Werror`) |
| cppcheck 2.13 | `warning`, `style`, `performance`, `portability` | 0 findings (11 `const` correctness findings fixed) | Fails on any finding |
| clang-tidy 22 | `clang-analyzer-*`, `bugprone-*`, `cert-*`, `misc-*` (exclusions below) | 0 findings (one possible out-of-bounds index made explicit, one implicit widening made explicit) | Fails on any finding |
| cppcheck MISRA C:2012 addon | Decidable MISRA C:2012 rules supported by the addon | 671 findings, all documented below (1132 before the fixes) | Report only |

### clang-tidy exclusions

| Check | Reason |
|:---|:---|
| `clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling` | Recommends the C11 Annex K `_s` functions, which newlib (Arm GNU toolchain) does not provide. Every `memset` / `memcpy` / `strncpy` call uses a length derived from `sizeof` of the destination. |
| `bugprone-reserved-identifier`, `cert-dcl37-c`, `cert-dcl51-cpp` | Only raised in `Tests/mock_hal.h`, which reproduces the `__HAL_RCC_*` macro names of the ST HAL. |
| `bugprone-easily-swappable-parameters` | The driver API follows the STM32Cube BSP signatures (address, buffer, size). |
| `misc-include-cleaner` | Include hygiene, not a defect class. |
| `bugprone-branch-clone` at `stm32n6_extmem.c` (`NOLINTNEXTLINE`) | The per-instance clock macros are empty in the host mock, so the branches look identical; they differ with the STM32CubeN6 HAL. |

## MISRA C:2012

The cppcheck addon checks a subset of the MISRA C:2012 rules and runs without the licensed rule texts, so
it reports rule numbers only. **This is not a MISRA compliance claim**: compliance requires a qualified
tool (for example PC-lint Plus, Polyspace, Helix QAC) covering every rule, plus a reviewed deviation record.

Fixed in the driver code (all host tests and 100% line and branch coverage kept):
- Rule 15.6 (326): every `if` / `else` / loop body is a compound statement.
- Rule 10.4 (78): unsigned suffixes on constants compared or combined with unsigned operands.
- Rule 17.7 (12): ignored return values of `memset` / `memcpy` / `strncpy` cast to `void`.
- Rule 15.7 (9): every `if ... else if` chain ends with an `else`.
- Rule 14.4 (7): controlling expressions are explicit comparisons.
- Rules 10.3, 10.7, 10.8 (6): essential type conversions made explicit or removed (`1UL` replaced by `1U`
  with a guarded shift, cast applied before the shift).
- Rule 2.2 (4): redundant `1 *` factors in the device table; Rule 8.9 (1): object moved to block scope.

Remaining findings and their justification:

| Rule | Category | Count | Justification |
|:---|:---|---:|:---|
| 15.5 (single point of exit) | Advisory | 500 | The drivers return as soon as a HAL call fails, as the STM32Cube BSP drivers do. A single exit would add a status variable and nesting to every function without changing the behaviour, and the fault injection tests prove every early return. |
| 12.1 (explicit precedence) | Advisory | 65 | Mostly comparisons combined with `&&` / `\|\|` and shifts inside bit masks, where C precedence is unambiguous. |
| 10.6 (composite expression assigned to a wider type) | Required | 26 | Addon false positives: the flagged expressions are conditional operators with simple operands (`x > 0U ? x : 6U`), which MISRA C:2012 does not define as composite expressions. |
| 17.8 (function parameter modified) | Advisory | 24 | Chunked transfer loops advance their `Address` / `pData` / `Size` parameters. |
| 12.2 (shift amount in range) | Required | 14 | Addon false positives on constant register masks (`1U << 15`, `~MASK`) whose shift amounts are constants below 32. |
| 11.6 (cast between pointer and integer) | Required | 12 | CMSIS peripheral pointers (`XSPI1`, `FMC_NORSRAM_DEVICE`) are integer addresses cast to pointers; unavoidable when using the ST HAL. |
| 10.3 (assignment to a narrower or different essential type) | Required | 10 | Addon false positives on pointer arithmetic (`pData += chunk`) and on `0xFF` assigned to `uint8_t`. |
| 18.4 (pointer arithmetic) | Advisory | 9 | Buffer walking in chunked transfers. |
| 12.3 (comma operator) | Advisory | 3 | Comma-separated declarations of related locals. |
| 13.3 (increment combined with other side effects) | Advisory | 3 | `i++` in loop expressions together with an array access. |
| 11.4 (pointer to integer conversion) | Advisory | 2 | Removing `const` through `uintptr_t` to share one transfer routine between read and write (the data is not modified on writes). |
| 10.7, 10.8 | Required | 2 | `(uint32_t)(1ULL << n)` in the SFDP density decoding, where the 64-bit shift is intended and range-checked, and a narrowing `uint16_t` cast of a register mask. |
| 8.11 (array size in extern declaration) | Required | 1 | `ExtMem_DeviceDatabase[]` is sized by its initialiser; its length is exported as `EXTMEM_DEVICE_DATABASE_SIZE`. |

The ST HAL itself and the host test harness (`Tests/`) are outside the analysed scope.
