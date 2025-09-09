#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../include/fat12.h"

// ANSI color codes for better output
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Global test counters
extern int tests_run;
extern int tests_passed;
extern int tests_failed;

// Test result macros
#define TEST_PASS(message) do { \
    printf(ANSI_COLOR_GREEN "✅ PASS: %s" ANSI_COLOR_RESET "\n", message); \
    tests_passed++; \
    tests_run++; \
} while(0)

#define TEST_FAIL(message, ...) do { \
    printf(ANSI_COLOR_RED "❌ FAIL: " message ANSI_COLOR_RESET "\n", ##__VA_ARGS__); \
    tests_failed++; \
    tests_run++; \
} while(0)

// Basic assertion macros
#define TEST_ASSERT(condition, message) do { \
    if (condition) { \
        TEST_PASS(message); \
    } else { \
        TEST_FAIL(message); \
    } \
} while(0)

#define TEST_ASSERT_EQ(expected, actual, message) do { \
    if ((expected) == (actual)) { \
        TEST_PASS(message); \
    } else { \
        TEST_FAIL("%s - Expected %d, got %d", message, (int)(expected), (int)(actual)); \
    } \
} while(0)

#define TEST_ASSERT_EQ_HEX(expected, actual, message) do { \
    if ((expected) == (actual)) { \
        TEST_PASS(message); \
    } else { \
        TEST_FAIL("%s - Expected 0x%X, got 0x%X", message, (unsigned)(expected), (unsigned)(actual)); \
    } \
} while(0)

#define TEST_ASSERT_NEQ(expected, actual, message) do { \
    if ((expected) != (actual)) { \
        TEST_PASS(message); \
    } else { \
        TEST_FAIL("%s - Values should not be equal: %d", message, (int)(actual)); \
    } \
} while(0)

#define TEST_ASSERT_NULL(ptr, message) do { \
    if ((ptr) == NULL) { \
        TEST_PASS(message); \
    } else { \
        TEST_FAIL("%s - Expected NULL, got %p", message, (void*)(ptr)); \
    } \
} while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) do { \
    if ((ptr) != NULL) { \
        TEST_PASS(message); \
    } else { \
        TEST_FAIL("%s - Expected non-NULL pointer", message); \
    } \
} while(0)

// Test suite macros
#define TEST_SUITE_START(name) do { \
    printf(ANSI_COLOR_BLUE "\n=== Test Suite: %s ===" ANSI_COLOR_RESET "\n", name); \
    tests_run = 0; \
    tests_passed = 0; \
    tests_failed = 0; \
} while(0)

#define TEST_SUITE_END(name) do { \
    printf(ANSI_COLOR_BLUE "\n=== %s Results ===" ANSI_COLOR_RESET "\n", name); \
    printf("Tests run: %d\n", tests_run); \
    printf(ANSI_COLOR_GREEN "Passed: %d" ANSI_COLOR_RESET "\n", tests_passed); \
    if (tests_failed > 0) { \
        printf(ANSI_COLOR_RED "Failed: %d" ANSI_COLOR_RESET "\n", tests_failed); \
    } \
    printf("Success rate: %.1f%%\n", tests_run > 0 ? (tests_passed * 100.0 / tests_run) : 0.0); \
    return tests_failed; \
} while(0)

// Helper function to create mock BootSector for testing
static inline void create_mock_boot_sector(BootSector* boot, 
                                         uint16_t bytes_per_sector,
                                         uint8_t sectors_per_cluster, 
                                         uint16_t reserved_sectors,
                                         uint8_t fat_count,
                                         uint16_t root_entries,
                                         uint16_t total_sectors,
                                         uint16_t sectors_per_fat) {
    boot->bytes_per_sector = bytes_per_sector;
    boot->sectors_per_cluster = sectors_per_cluster;
    boot->reserved_sectors = reserved_sectors;
    boot->fat_count = fat_count;
    boot->root_entries = root_entries;
    boot->total_sectors = total_sectors;
    boot->sectors_per_fat = sectors_per_fat;
}

#endif // TEST_COMMON_H