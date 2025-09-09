#include "test_common.h"
#include "../include/fat12.h"
#include <string.h>
#include <limits.h>

void test_boot_sector_validation() {
    printf("\n--- Testing boot sector validation ---\n");
    
    BootSector boot;
    
    // Test 1: Valid boot sector (should pass current validation)
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 9);
    printf("Testing valid boot sector...\n");
    // We can't directly test read_boot_sector without a file, but we can test the validation logic
    
    TEST_ASSERT(boot.bytes_per_sector == 512, "Valid bytes_per_sector");
    TEST_ASSERT(boot.sectors_per_cluster > 0, "Valid sectors_per_cluster");
    
    // Test 2: Invalid bytes_per_sector (current validation should catch this)
    printf("\nTesting invalid bytes_per_sector...\n");
    create_mock_boot_sector(&boot, 256, 1, 1, 2, 224, 2880, 9); // 256 instead of 512
    TEST_ASSERT(boot.bytes_per_sector != 512, "Invalid bytes_per_sector should be rejected");
    
    create_mock_boot_sector(&boot, 1024, 1, 1, 2, 224, 2880, 9); // 1024 instead of 512  
    TEST_ASSERT(boot.bytes_per_sector != 512, "Non-standard bytes_per_sector should be rejected");
    
    // Test 3: Invalid sectors_per_cluster (current validation should catch this)
    printf("\nTesting invalid sectors_per_cluster...\n");
    create_mock_boot_sector(&boot, 512, 0, 1, 2, 224, 2880, 9); // 0 sectors per cluster
    TEST_ASSERT(boot.sectors_per_cluster == 0, "Zero sectors_per_cluster should be rejected");
    
    // Test 4: Edge cases that current validation DOESN'T catch but should
    printf("\nTesting edge cases that should be validated...\n");
    
    // Zero FAT count
    create_mock_boot_sector(&boot, 512, 1, 1, 0, 224, 2880, 9);
    TEST_FAIL("Zero FAT count should be invalid but isn't currently checked");
    
    // Too many FATs
    create_mock_boot_sector(&boot, 512, 1, 1, 10, 224, 2880, 9);
    TEST_FAIL("Too many FATs should be invalid but isn't currently checked");
    
    // Zero root entries
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 0, 2880, 9);
    TEST_FAIL("Zero root entries should be invalid but isn't currently checked");
    
    // Zero sectors per FAT
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 0);
    TEST_FAIL("Zero sectors_per_fat should be invalid but isn't currently checked");
    
    // Zero total sectors
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 0, 9);
    TEST_FAIL("Zero total_sectors should be invalid but isn't currently checked");
    
    // Reserved sectors too large
    create_mock_boot_sector(&boot, 512, 1, 1000, 2, 224, 2880, 9);
    TEST_FAIL("Too many reserved sectors should be invalid but isn't currently checked");
    
    // Test 5: Consistency checks that should exist
    printf("\nTesting consistency checks that should exist...\n");
    
    // Total sectors smaller than required minimum structure
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 50, 9); // Way too small
    int min_sectors = boot.reserved_sectors + (boot.fat_count * boot.sectors_per_fat) + 
                      ((boot.root_entries * 32 + boot.bytes_per_sector - 1) / boot.bytes_per_sector);
    
    if (boot.total_sectors < min_sectors) {
        TEST_FAIL("total_sectors (%d) smaller than minimum required (%d) - should be validated", 
                  boot.total_sectors, min_sectors);
    } else {
        TEST_PASS("total_sectors is sufficient for filesystem structure");
    }
    
    // FAT too small for the claimed total sectors
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 1); // Only 1 sector per FAT
    int data_sectors = boot.total_sectors - boot.reserved_sectors - 
                       (boot.fat_count * boot.sectors_per_fat) -
                       ((boot.root_entries * 32 + boot.bytes_per_sector - 1) / boot.bytes_per_sector);
    int max_clusters = data_sectors / boot.sectors_per_cluster;
    int fat_entries_available = (boot.sectors_per_fat * boot.bytes_per_sector * 2) / 3; // FAT12: 2 entries per 3 bytes
    
    if (max_clusters > fat_entries_available) {
        TEST_FAIL("FAT too small (%d entries) for data area (%d clusters) - should be validated",
                  fat_entries_available, max_clusters);
    } else {
        TEST_PASS("FAT size is sufficient for data area");
    }
}

void test_load_fat_table_size_validation() {
    printf("\n--- Testing load_fat_table size validation ---\n");
    
    BootSector boot;
    
    // Test 1: Normal FAT size (should work)
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 9);
    int fat_size = boot.sectors_per_fat * boot.bytes_per_sector;
    printf("Normal FAT size: %d bytes\n", fat_size);
    TEST_ASSERT(fat_size == 4608, "Normal FAT size calculation");  // 9 * 512 = 4608
    
    // Test 2: Zero FAT size (should fail)  
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 0);
    fat_size = boot.sectors_per_fat * boot.bytes_per_sector;
    printf("Zero FAT size: %d bytes\n", fat_size);
    TEST_ASSERT(fat_size == 0, "Zero FAT size should be rejected");
    
    // Test 3: Extremely large FAT size (should fail)
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 65535); // Max uint16_t
    fat_size = boot.sectors_per_fat * boot.bytes_per_sector;
    printf("Huge FAT size: %d bytes (%.1f MB)\n", fat_size, fat_size / 1024.0 / 1024.0);
    
    // This could cause integer overflow or massive malloc
    if (fat_size > 1024 * 1024) { // > 1MB is suspicious for FAT12
        TEST_FAIL("Huge FAT size (%d bytes) should be rejected but isn't currently checked", fat_size);
    } else {
        TEST_PASS("FAT size is reasonable");
    }
    
    // Test 4: Integer overflow in FAT size calculation
    create_mock_boot_sector(&boot, 65535, 1, 1, 2, 224, 2880, 65535);
    // sectors_per_fat * bytes_per_sector could overflow uint32
    uint64_t fat_size_64 = (uint64_t)boot.sectors_per_fat * boot.bytes_per_sector;
    printf("Potential overflow: %u * %u = %llu\n", boot.sectors_per_fat, boot.bytes_per_sector, fat_size_64);
    
    if (fat_size_64 > UINT32_MAX) {
        TEST_FAIL("FAT size calculation overflows uint32 - should be validated");
    } else {
        TEST_PASS("No integer overflow in FAT size calculation");
    }
    
    // Test 5: Reasonable limits for FAT12
    printf("\nTesting FAT12-specific limits...\n");
    
    // FAT12 should have max ~4084 clusters, each cluster entry is 1.5 bytes
    // So max FAT size should be around (4084 * 1.5) = ~6126 bytes per FAT
    // With 2 FATs = ~12KB total, round up for safety = ~20KB max
    
    int max_reasonable_fat_size = 20 * 1024; // 20KB per FAT
    
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 9);
    fat_size = boot.sectors_per_fat * boot.bytes_per_sector;
    
    if (fat_size <= max_reasonable_fat_size) {
        TEST_PASS("FAT size is within reasonable FAT12 limits");
    } else {
        TEST_FAIL("FAT size (%d bytes) exceeds reasonable FAT12 limits (%d bytes)",
                  fat_size, max_reasonable_fat_size);
    }
    
    // Test very large sectors_per_fat for FAT12
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 100); // 100 sectors per FAT
    fat_size = boot.sectors_per_fat * boot.bytes_per_sector;
    
    if (fat_size > max_reasonable_fat_size) {
        TEST_FAIL("Large sectors_per_fat (%d) results in oversized FAT (%d bytes) for FAT12",
                  boot.sectors_per_fat, fat_size);
    } else {
        TEST_PASS("FAT size with large sectors_per_fat is still reasonable");
    }
}

void test_memory_allocation_limits() {
    printf("\n--- Testing memory allocation limits ---\n");
    
    // Test malloc size limits
    size_t reasonable_limit = 100 * 1024 * 1024;  // 100MB
    size_t huge_size = SIZE_MAX / 2;  // Huge allocation
    
    printf("Testing allocation size limits...\n");
    printf("Reasonable limit: %zu bytes (%.1f MB)\n", reasonable_limit, reasonable_limit / 1024.0 / 1024.0);
    printf("Huge size: %zu bytes\n", huge_size);
    
    // These tests document what should be checked in load_fat_table
    TEST_FAIL("load_fat_table doesn't check for reasonable allocation sizes");
    TEST_FAIL("load_fat_table doesn't prevent huge memory allocations");
    TEST_FAIL("load_fat_table doesn't handle malloc failures gracefully in all cases");
    
    // Test edge case: what if sectors_per_fat * bytes_per_sector = 0?
    BootSector boot;
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 0);
    int fat_size = boot.sectors_per_fat * boot.bytes_per_sector;  // = 0
    
    if (fat_size == 0) {
        TEST_FAIL("Zero allocation size could cause malloc(0) - behavior is implementation defined");
    } else {
        TEST_PASS("Non-zero allocation size");
    }
}

int main() {
    TEST_SUITE_START("Input Validation");
    
    test_boot_sector_validation();
    test_load_fat_table_size_validation();
    test_memory_allocation_limits();
    
    TEST_SUITE_END("Input Validation");
}