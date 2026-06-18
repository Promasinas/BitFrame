# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

BitFream is a C/C++ system in early development. This `agent/` subdirectory is the working root for agent-related code. The parent project (`I:\Develop\BitFream`) follows a standard C/C++ layout: `src/`, `include/`, `lib/`, `build/`, `bin/`.

## Build System

No build system (Makefile, CMake) exists yet. The project layout anticipates one — add build configuration to `build/`, compile outputs to `bin/`, and libraries to `lib/`.

## Architecture

The system is organized into several modules under `src/`:

- **`src/MemoryManagement/`** — Core memory subsystem. The only module with implementation so far.
  - `inc/main_memory.h` — Public header declaring the global `uint64_t* main_memory_block` and `uint64_t main_memory_block_size`, plus `set_main_memory_block_size()` and `clear_main_memory_block()`.
  - `src/main_memory.c` — Implementation (currently empty stub).
  - `MemoryManagement.h` — Module umbrella header.

- **`src/OperatorKernals/`** — Operator kernel implementations (empty; `OperatorKernals.h` header exists).

- **`src/RuntimeManagement/`** — Runtime lifecycle management (empty; `RuntimeManagement.h` header exists with empty `inc/` and `src/`).

- **`src/LanguageWrapper/`** — Language bindings, with empty `Python/` and `Lua/` subdirectories.

- **`src/API/`** — Public API surface (empty).

- **`src/Utils/`** — Shared utilities (empty).

- **`agent/`** — This directory; intended for agent-specific code (currently empty).

## Coding Conventions

- **Header guards**: `__MODULE_NAME_H__` pattern (double underscore prefix/suffix).
- **Public headers** go in `inc/` subdirectories within each module.
- **Implementation** goes in `src/` subdirectories within each module.
- **Types**: Uses `<cstdint>` (C++ header) for fixed-width integers, though source files are `.c` — this may indicate a mixed C/C++ codebase or an early inconsistency. Clarify with the team whether the project targets C or C++.
- Module umbrella headers (e.g., `MemoryManagement.h`) live at the module root and include the relevant sub-headers.
