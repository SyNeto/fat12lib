# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C library (`fat12lib`) for reading and analyzing FAT12 disk images. The library provides utilities to:
- Parse FAT12 boot sectors
- Analyze File Allocation Table (FAT) structures
- Extract disk usage statistics (total, used, and free clusters)

## Architecture

The codebase follows a simple library structure:

- `include/fat12.h` - Public API definitions with `BootSector` and `DiskInfo` structures
- `src/fat12.c` - Implementation of FAT12 parsing and analysis functions
- Core functions: `read_boot_sector()`, `analyze_fat()`, `print_disk_info()`

Key data structures:
- `BootSector`: Contains FAT12 filesystem metadata (sectors, clusters, FAT count)
- `DiskInfo`: Aggregates boot sector data with computed cluster statistics

## Build Commands

```bash
# Build static library
make all
make libfat12.a

# Clean build artifacts  
make clean
```

The build system uses:
- GCC with `-Wall -Wextra -std=c99` flags  
- Organized build directory structure:
  - `build/obj/` - Object files (.o)
  - `build/lib/` - Static library (`libfat12.a`)
- Clean separation between source and build artifacts
- No test framework currently configured

## Code Conventions

- C99 standard compliance
- Structured types for FAT12 data representation
- Macro definitions for filesystem layout calculations (FAT_START, ROOT_DIR_START, DATA_START)
- Spanish language output in `print_disk_info()` function
- 32-byte directory entry size assumption for root directory calculations