# BitFream

High-performance C library for 1-bit boolean convolution, designed for
binary neural network (BNN) inference and training.

**Key features:**
- Packed 1-bit data (8 bits/byte, LSB-first)
- XOR + hardware POPCNT acceleration
- Contiguous-memory fast path for dense (dilation=1) convolutions
- Input upsampling via zero-insertion
- Cross-platform shared libraries (Windows / Linux / macOS)

## Project Structure

```
BitFream/
├── src/
│   ├── MemoryManagement/        # Block-based memory allocator
│   ├── OperatorKernals/         # 1-bit conv operators (XOR-accelerated)
│   ├── RuntimeManagement/       # Sequential callback chains
│   ├── Utils/                   # Logging initialisation
│   └── main.c                   # Entry point
├── lib/
│   └── log.c/                   # Third-party: rxi/log.c (MIT)
├── test/                        # Unit tests — mirrors src/ & lib/
├── build/                       # CMake output (.dll, .a, .exe)
└── logs/                        # Runtime log files
```

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Options**

| Option | Default | Description |
|---|---|---|
| `BUILD_TESTS` | `ON` | Build test executables |
| `BUILD_MAIN` | `ON` | Build main executable |
| `USE_POPCNT` | `ON` | Hardware POPCNT (x86) |
| `USE_AVX2` | `OFF` | AVX2 instruction set |

**Platforms:** Windows (MSVC / MinGW), Linux (GCC / Clang), macOS (Clang).

## Test

Each test is compiled and run independently:

```bash
# Build all test targets
cmake --build build --target test_bool_conv_1d test_bool_conv_1d_upsample \
                             test_main_memory test_log_init test_log \
                             test_network_forward test_network_testlin_train

# Run (Windows: add DLL directory to PATH)
export PATH="$PWD/build:$PATH"

./test/OperatorKernals/bool_conv_1d/build/test_bool_conv_1d.exe
./test/OperatorKernals/bool_conv_1d_upsample/build/test_bool_conv_1d_upsample.exe
./test/MemoryManagement/main_memory/build/test_main_memory.exe
./test/Utils/log_init/build/test_log_init.exe
./test/lib/log/build/test_log.exe
./test/RuntimeManagement/network_forward/build/test_network_forward.exe
./test/RuntimeManagement/network_testlin_train/build/test_network_testlin_train.exe

# Or via CTest
cd build && ctest
```

## Modules

| Module | Library | Public API |
|---|---|---|
| `OperatorKernals` | `libOperatorKernals` | `bool_conv_1d_forward`, `bool_conv_1d_upsample_forward` |
| `MemoryManagement` | `libMemoryManagement` | `add_block`, `activate_blocks`, `get_block_by_index`, `clear_blocks`, `clear_main_memory` |
| `RuntimeManagement` | `libRuntimeManagement` | `bf_forward_register/execute/clear`, `bf_train_register/execute/clear` |
| `Utils` | `libUtils` | `log_init`, `log_shutdown` |

Each module has its own **independent** header and source files with no cross-dependencies.
See `src/OperatorKernals/OperatorKernals.h` (or each module's umbrella header) for a single-include entry point.

### How it works

```
Input (packed bits)    Kernel (packed bits)
   [1 0 1 1 0 ...]       [1 0 1]
         │                    │
         ▼                    ▼
   ┌──────────┐       ┌──────────────┐
   │  gather  │       │  pre-packed  │
   │  window  │       │  uint64_t[]  │
   └────┬─────┘       └──────┬───────┘
        │                    │
        └────── XOR ─────────┘
                  │
                  ▼
          POPCNT (hardware)
                  │
                  ▼
       output = kernel_size − mismatches
```

- **dilation=1**: window is extracted via contiguous memory copy (memcpy or shifted copy)
- **dilation>1**: bits are gathered with stride, then packed for XOR
- **upsample**: zeros are inserted between input bits before convolution

## License

BitFream is licensed under the **Apache License, Version 2.0**.
See [LICENSE](LICENSE) for the full text.

Third-party: [rxi/log.c](https://github.com/rxi/log.c) is licensed under **MIT**.
See [lib/log.c/LICENSE](lib/log.c/LICENSE).
