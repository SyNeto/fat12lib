# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C library (`fat12lib`) for reading, analyzing, and manipulating FAT12 disk images. The library provides utilities to:
- Parse FAT12 boot sectors with correct offset handling
- Analyze File Allocation Table (FAT) structures with bounds checking
- Dynamic offset calculation for all disk sections (no hardcoded values)
- Cluster-to-offset conversion with full FAT12 validation (range 2-4084)
- Extract disk usage statistics (total, used, and free clusters)
- Safe 12-bit FAT entry reading and writing
- Cross-platform compatibility (Linux, macOS, Windows/MSVC)

## Architecture

The codebase follows a modular library structure:

- `include/fat12.h` - Public API definitions with `BootSector` and `DiskInfo` structures
- `src/fat12.c` - Implementation of FAT12 parsing and analysis functions
- `src/fat12_layout.c` - Dynamic offset calculation functions (NEW)
- Core functions:
  - Boot sector: `read_boot_sector()`
  - FAT operations: `load_fat_table()`, `get_fat12_entry()`, `set_fat12_entry()`
  - Disk analysis: `analyze_fat()`, `print_disk_info()`
  - Layout calculations: `calculate_root_directory_offset()`, `calculate_root_directory_size()`, `calculate_data_area_offset()`, `cluster_to_offset()`
- Comprehensive test suite with 88 tests achieving 100% pass rate

Key data structures:
- `BootSector`: Contains FAT12 filesystem metadata (sectors, clusters, FAT count)
- `DiskInfo`: Aggregates boot sector data with computed cluster statistics

## Build Commands

```bash
# Build static library
make all
make libfat12.a

# Run comprehensive tests
make test

# Clean build artifacts  
make clean
```

The build system uses:
- C17-compliant compiler (GCC, Clang, MSVC) with `-Wall -Wextra -std=c17` flags  
- Organized build directory structure:
  - `build/obj/` - Object files (.o)
  - `build/lib/` - Static library (`libfat12.a`)
  - `build/tests/` - Test executables
- Clean separation between source and build artifacts
- Comprehensive test framework with 88 tests across 6 categories

## Code Conventions

- C17 standard compliance with cross-platform compatibility
- Structured types for FAT12 data representation with `__attribute__((packed))`
- **Dynamic offset functions** for layout calculations (replacing legacy macros)
  - Legacy macros (FAT_START, ROOT_DIR_START, DATA_START) are **deprecated**
  - New functions: `calculate_root_directory_offset()`, `calculate_data_area_offset()`, `cluster_to_offset()`
  - Helper macro: `CLUSTER_SIZE(boot)` for cluster size calculations
- Dynamic memory allocation with goto cleanup pattern for resource management
- Comprehensive bounds checking and input validation
- English language output in all functions
- 32-byte directory entry size assumption for root directory calculations

## Memory Management

- **Dynamic Allocation**: malloc/free used consistently (no VLA for MSVC compatibility)
- **Goto Cleanup Pattern**: Consistent resource cleanup on all exit paths
- **Bounds Checking**: All array access and memory operations are validated
- **Error Handling**: Specific error codes for different failure modes

## Contribution Guidelines

This project follows an **issue-driven development** approach:

### Vibe Coding Policy
- **Vibe coding is permitted** for exploration and prototyping
- **However**: Every piece of code must be thoroughly analyzed before integration
- All additions require:
  - Code review and approval
  - Comprehensive testing (maintain 100% pass rate)
  - Documentation updates
  - Security analysis

### Development Workflow
1. **Find or create an issue** describing the feature/bug
2. **Get issue prioritized** by maintainers  
3. **Fork and create feature branch**
4. **Implement with tests** - maintain 100% test coverage
5. **Submit Pull Request** with clear description and test results

### Code Quality Standards
- **Security First**: All code must be memory-safe and bounds-checked
- **Test Coverage**: New code requires comprehensive test coverage
- **Documentation**: Public APIs must be documented
- **Cross-Platform**: Must work on Linux, macOS, Windows (GCC, Clang, MSVC)

## Project Status

### ✅ **Phase 1: Read & Analysis (Complete)**
- Boot sector parsing with correct offset handling (fixed struct alignment issues)
- FAT table operations with bounds checking
- **Dynamic offset calculation functions** (NEW)
  - `calculate_root_directory_offset()` - Root directory location
  - `calculate_root_directory_size()` - Root directory size in bytes
  - `calculate_data_area_offset()` - Data area starting offset
  - `cluster_to_offset()` - Convert cluster number to physical offset
  - Full validation (NULL checks, cluster range 2-4084)
- Disk analysis and statistics
- Comprehensive test suite (88 tests, 100% pass rate)
- Cross-platform compatibility including MSVC

### 🔄 **Phase 2: Directory & File Operations (Next)**
- Root directory reading functionality
- Directory entry structure and parsing
- File content reading by following cluster chains
- High-level file API

### 📋 **Phase 3: File Manipulation (Planned)**
- File creation and deletion
- Directory creation and removal
- Free space management

## Important Notes for Claude

- The library currently focuses on **reading and analysis** - modification capabilities are planned
- All FAT12 operations use **correct offset handling** (boot sector fields start at 0x0B, not 0x00)
- The `BootSector` struct uses `__attribute__((packed))` but we read from correct offsets manually
- **Dynamic offset calculation** functions replace legacy hardcoded macros
  - Legacy macros (FAT_START, ROOT_DIR_START, DATA_START) are deprecated
  - Use new functions: `calculate_root_directory_offset()`, `calculate_data_area_offset()`, `cluster_to_offset()`
  - These functions support **any valid FAT12 configuration** (not just 1.44MB floppies)
- Memory management follows **goto cleanup pattern** for consistency and MSVC compatibility
- When adding new features, maintain the established patterns for bounds checking and error handling
- The offset calculation functions have **zero dynamic memory allocation** (stack-only calculations)