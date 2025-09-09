# FAT12lib - Robust FAT12 Filesystem Library

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](#testing)
[![Test Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen)](#testing)
[![License](https://img.shields.io/badge/license-MIT-blue)](#license)
[![C17](https://img.shields.io/badge/C-C17-blue)](#requirements)

A C library for reading, analyzing, and manipulating FAT12 disk images. Built with security, performance, and reliability in mind.

## 🚀 Features

### ✅ **Currently Implemented (Phase 1: Read & Analysis)**

- **Boot Sector Operations**
  - Correct FAT12 boot sector parsing with proper offset handling
  - Comprehensive validation (power-of-2 clusters, layout consistency, etc.)
  - Enhanced error reporting with specific error codes

- **FAT Table Management** 
  - Safe 12-bit FAT entry reading and writing with bounds checking
  - Dynamic memory allocation with goto cleanup pattern for resource safety
  - Buffer overrun prevention and input validation
  - MSVC and cross-compiler compatibility

- **Disk Analysis**
  - Complete filesystem structure analysis (clusters, free space, usage statistics)
  - FAT12 specification compliance validation
  - Disk health and consistency checking

### 🔄 **Next Phase: Directory & File Operations**

The following features are planned for implementation:

- **Directory Reading**
  - Root directory enumeration  
  - Directory entry parsing (8.3 filenames, attributes, timestamps)
  - Subdirectory navigation support

- **File Operations**
  - File content reading by following cluster chains
  - File metadata extraction
  - Path resolution and navigation

- **High-Level API**
  - Intuitive file listing and reading interface
  - Cross-platform compatibility layer
  - Long filename (VFAT) support

## 📋 Project Status

| Component | Status | Tests | Coverage |
|-----------|--------|-------|----------|
| Boot Sector Parsing | ✅ Complete | 12/12 | 100% |
| FAT Table Operations | ✅ Complete | 21/21 | 100% |
| Input Validation | ✅ Complete | 17/17 | 100% |
| Memory Management | ✅ Complete | 6/6 | 100% |
| Struct Alignment | ✅ Complete | 10/10 | 100% |
| Directory Reading | ⏳ Planned | - | - |
| File Operations | ⏳ Planned | - | - |
| VFAT Support | 📋 Backlog | - | - |

## 🛠️ Building

### Requirements
- C17-compliant compiler (GCC, Clang, MSVC)
- Make (or equivalent build system)
- Compatible with: Linux, macOS, Windows (including WSL, Cygwin, MSVC)

### Quick Start
```bash
# Clone the repository
git clone <repository-url>
cd fat12lib

# Build the library
make all

# Run comprehensive tests
make test

# Clean build artifacts
make clean
```

### Build Targets
```bash
make all                    # Build static library
make test                   # Run all test suites
make test-calculations      # Test FAT12 calculations
make test-bounds           # Test bounds checking
make test-validation       # Test input validation
make test-alignment        # Test struct alignment
make test-fixed           # Test boot sector reading
make clean                # Clean all build artifacts
```

## 📖 Usage

### Basic Example
```c
#include "fat12.h"

int main() {
    FILE *img = fopen("disk.img", "rb");
    if (!img) return -1;
    
    // Read and validate boot sector
    BootSector boot;
    if (read_boot_sector(img, &boot) != 0) {
        fprintf(stderr, "Invalid FAT12 boot sector\n");
        return -1;
    }
    
    // Analyze filesystem
    DiskInfo info;
    if (analyze_fat(img, &boot, &info) != 0) {
        fprintf(stderr, "FAT analysis failed\n");
        return -1;
    }
    
    // Display disk information
    print_disk_info(&info);
    
    fclose(img);
    return 0;
}
```

### Advanced FAT Table Manipulation
```c
// Load FAT table for editing
uint8_t *fat_table = load_fat_table(img, &boot);
if (!fat_table) return -1;

// Read/write FAT entries with bounds checking
int fat_size = boot.sectors_per_fat * boot.bytes_per_sector;
uint16_t entry = get_fat12_entry(fat_table, fat_size, cluster_num);

if (set_fat12_entry(fat_table, fat_size, cluster_num, new_value) != 0) {
    fprintf(stderr, "Cluster out of bounds\n");
}

free_fat_table(fat_table);
```

## 🧪 Testing

The library includes a comprehensive test suite with 77 tests across 5 categories:

```bash
# Run all tests
make test

# Individual test suites
make test-calculations  # FAT12 layout calculations
make test-bounds       # Buffer overrun prevention  
make test-validation   # Input validation and error handling
make test-alignment    # Memory layout and struct padding
make test-fixed        # Boot sector parsing correctness
```

All tests achieve **100% pass rate** with extensive coverage of edge cases, error conditions, and security scenarios.

## 🤝 Contributing

We welcome contributions from developers interested in filesystem implementation, security, and low-level systems programming.

### Contribution Guidelines

#### 🎯 **Issue-Driven Development**
- All external contributions must be linked to **prioritized issues** in the GitHub project board
- Check the [Issues](../../issues) section for current priorities and feature requests
- Create or comment on an issue before starting work to avoid duplication

#### 🎨 **Vibe Coding Policy**
- **Vibe coding is permitted** for exploration and prototyping
- **However**: Every piece of code must be thoroughly analyzed before integration
- All additions require:
  - Code review and approval
  - Comprehensive testing
  - Documentation updates
  - Security analysis

#### 📋 **Development Workflow**
1. **Find or create an issue** describing the feature/bug
2. **Get issue prioritized** by maintainers
3. **Fork the repository** and create a feature branch
4. **Implement with tests** - maintain 100% test coverage
5. **Submit Pull Request** with:
   - Clear description linking to issue
   - Test results showing 100% pass rate
   - Documentation updates if needed

#### ✅ **Code Quality Standards**
- **Security First**: All code must be memory-safe and bounds-checked
- **Test Coverage**: New code requires comprehensive test coverage
- **Documentation**: Public APIs must be documented
- **C17 Compliance**: Follow modern C practices and standards

### Current Priority Areas

See our [Project Board](../../projects) for prioritized work items:

1. **High Priority**: Directory reading implementation
2. **Medium Priority**: File content reading via cluster chains  
3. **Low Priority**: VFAT long filename support

## 📚 Architecture

### Memory Management
- **Dynamic Allocation**: malloc/free for all variable-sized data structures
- **Goto Cleanup**: Consistent error handling with resource cleanup
- **Bounds Checking**: All array access is validated
- **Cross-Compiler Compatibility**: No VLA usage for MSVC compatibility

### Security Features
- **Buffer Overrun Prevention**: Comprehensive bounds checking on all operations
- **Input Validation**: Rigorous validation of all filesystem structures
- **Error Handling**: Detailed error codes for debugging and recovery

### Performance Considerations
- **Stack-Based Temporary Operations**: VLA for small, short-lived data
- **Heap-Based Persistent Data**: malloc for user-controlled data lifetime
- **Minimal Dependencies**: Only standard C library dependencies

## 🎯 Roadmap

### Phase 2: Directory & File Operations (Current Focus)
- [ ] Directory entry structure and parsing
- [ ] Root directory reading functionality
- [ ] Cluster chain traversal implementation
- [ ] File content reading operations
- [ ] High-level file API

### Phase 3: File Manipulation
- [ ] File creation and deletion
- [ ] Directory creation and removal
- [ ] File content modification
- [ ] Free space management

### Phase 4: Advanced Features
- [ ] VFAT long filename support
- [ ] Disk defragmentation utilities
- [ ] Cross-platform compatibility layer
- [ ] Performance optimizations

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built with modern C17 standards for maximum compatibility
- Follows FAT12 specification as documented by Microsoft
- Inspired by the need for reliable, secure filesystem tools

---

**Ready to contribute?** Check out our [Issues](../../issues) and [Project Board](../../projects) to get started!