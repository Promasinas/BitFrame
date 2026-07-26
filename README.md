# BitFream

1-bit boolean convolution framework — high-performance C library.

## Project Structure

```
BitFream/
├── src/
│   ├── MemoryManagement/       # Block-based memory allocator
│   ├── OperatorKernals/        # 1-bit conv operators (XOR-accelerated)
│   ├── RuntimeManagement/      # Sequential callback chains
│   ├── Utils/                  # Logging initialisation
│   └── main.c                  # Entry point
├── lib/
│   └── log.c/                  # rxi/log.c (MIT)
├── test/                       # Unit tests (mirrors src/ & lib/)
├── bin/                        # Runtime output — .dll / .exe
├── lib/                        # Link-time output — .dll.a / .so / .dylib
├── build/                      # CMake build tree
└── logs/                       # Runtime log files
```

## Build

```bash
# Configure (shared libraries)
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build all
cmake --build build

# Options
cmake -B build -DBUILD_TESTS=OFF     # skip tests
cmake -B build -DBUILD_MAIN=OFF      # skip main executable
cmake -B build -DUSE_POPCNT=OFF      # disable hardware popcount
cmake -B build -DUSE_AVX2=ON         # enable AVX2
```

Cross-platform: Windows (MSVC / MinGW), Linux (GCC / Clang), macOS (Clang).

## Test

```bash
# Build & run all tests
cmake --build build --target test_bool_conv_1d test_bool_conv_1d_upsample \
                             test_main_memory test_log_init test_log \
                             test_network_forward test_network_testlin_train

# Or use CTest
cd build && ctest

# Or run individually
./test/OperatorKernals/bool_conv_1d/build/test_bool_conv_1d.exe
```

## Modules

| Module | DLL / .so | Description |
|---|---|---|
| `OperatorKernals` | `libOperatorKernals` | `bool_conv_1d_forward`, `bool_conv_1d_upsample_forward` |
| `MemoryManagement` | `libMemoryManagement` | Block allocator with `add_block` / `activate_blocks` |
| `RuntimeManagement` | `libRuntimeManagement` | Callback chains for forward & training pipelines |
| `Utils` | `libUtils` | `log_init` / `log_shutdown` |

## License

BitFream is licensed under the **Apache License, Version 2.0**. See [LICENSE](LICENSE) for the full text.

This project includes [rxi/log.c](https://github.com/rxi/log.c) which is licensed under the **MIT** license. See [lib/log.c/LICENSE](lib/log.c/LICENSE) for details.
