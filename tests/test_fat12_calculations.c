#include "test_common.h"
#include "../include/fat12.h"

// Need access to internal macros for testing
#define BOOT_SECTOR_OFFSET 0
#define FAT_START(boot) (BOOT_SECTOR_OFFSET + (boot->reserved_sectors * boot->bytes_per_sector))
#define ROOT_DIR_START(boot) (FAT_START(boot) + (boot->fat_count * boot->sectors_per_fat * boot->bytes_per_sector))
#define DATA_START(boot) (ROOT_DIR_START(boot) + (boot->root_entries * 32 / boot-> bytes_per_sector))

void test_data_start_macro() {
    BootSector boot;
    
    printf("\n--- Testing DATA_START macro ---\n");
    
    // Test case 1: Standard floppy (should work correctly)
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 9);
    
    int fat_start = FAT_START((&boot));
    int root_dir_start = ROOT_DIR_START((&boot));  
    int data_start = DATA_START((&boot));
    
    printf("FAT_START: %d bytes\n", fat_start);
    printf("ROOT_DIR_START: %d bytes\n", root_dir_start);
    printf("DATA_START: %d bytes\n", data_start);
    
    // Calculate expected values manually
    int expected_fat_start = 512;  // 1 reserved sector * 512
    int expected_root_start = 512 + (2 * 9 * 512); // FAT + (2 FATs * 9 sectors * 512)
    int expected_data_start = expected_root_start + (224 * 32); // Root + (224 entries * 32 bytes)
    
    TEST_ASSERT_EQ(expected_fat_start, fat_start, "FAT_START calculation");
    TEST_ASSERT_EQ(expected_root_start, root_dir_start, "ROOT_DIR_START calculation");
    
    // This test should FAIL with current implementation due to truncation bug
    TEST_ASSERT_EQ(expected_data_start, data_start, "DATA_START calculation (this may fail due to truncation bug)");
    
    // Test case 2: Edge case with truncation - should expose the bug
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 100, 2880, 9); // 100 root entries
    
    data_start = DATA_START((&boot));
    root_dir_start = ROOT_DIR_START((&boot));
    
    // Manual calculation: 100 * 32 = 3200 bytes = 6.25 sectors
    // Current macro: 3200 / 512 = 6 sectors (truncated) = 3072 bytes ❌
    // Correct: should be 7 sectors = 3584 bytes ✅
    
    int root_bytes = 100 * 32; // 3200 bytes
    int root_sectors_truncated = root_bytes / 512; // 6 sectors (wrong)
    int root_sectors_correct = (root_bytes + 511) / 512; // 7 sectors (correct ceiling)
    
    int data_start_wrong = root_dir_start + root_sectors_truncated * 512;
    int data_start_correct = root_dir_start + root_sectors_correct * 512;
    
    printf("\nTruncation test case:\n");
    printf("Root bytes: %d\n", root_bytes);
    printf("Root sectors (truncated): %d\n", root_sectors_truncated);
    printf("Root sectors (correct): %d\n", root_sectors_correct);
    printf("Current DATA_START: %d\n", data_start);
    printf("Expected (wrong): %d\n", data_start_wrong);
    printf("Expected (correct): %d\n", data_start_correct);
    
    // This should fail with current buggy implementation
    TEST_ASSERT_NEQ(data_start_correct, data_start, "DATA_START should be buggy (truncated) - this test documents the bug");
}

void test_data_start_sectors_calculation() {
    BootSector boot;
    
    printf("\n--- Testing data_start_sectors calculation ---\n");
    
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 9);
    
    // This mimics the buggy calculation in analyze_fat
    int data_start_bytes = DATA_START((&boot));
    int data_start_sectors_buggy = data_start_bytes / boot.bytes_per_sector;
    
    // Correct calculation should be based on sector arithmetic
    int fat_sectors = boot.fat_count * boot.sectors_per_fat;
    int root_bytes = boot.root_entries * 32;
    int root_sectors = (root_bytes + boot.bytes_per_sector - 1) / boot.bytes_per_sector; // Ceiling division
    int data_start_sectors_correct = boot.reserved_sectors + fat_sectors + root_sectors;
    
    printf("DATA_START (bytes): %d\n", data_start_bytes);
    printf("data_start_sectors (buggy method): %d\n", data_start_sectors_buggy);
    printf("data_start_sectors (correct method): %d\n", data_start_sectors_correct);
    
    // Document the double-truncation bug
    TEST_ASSERT_NEQ(data_start_sectors_correct, data_start_sectors_buggy, 
                   "Current calculation should be wrong due to double truncation");
    
    // Show the impact
    int total_data_sectors_buggy = boot.total_sectors - data_start_sectors_buggy;
    int total_data_sectors_correct = boot.total_sectors - data_start_sectors_correct;
    int total_clusters_buggy = total_data_sectors_buggy / boot.sectors_per_cluster;
    int total_clusters_correct = total_data_sectors_correct / boot.sectors_per_cluster;
    
    printf("Total data sectors (buggy): %d\n", total_data_sectors_buggy);
    printf("Total data sectors (correct): %d\n", total_data_sectors_correct);
    printf("Total clusters (buggy): %d\n", total_clusters_buggy);
    printf("Total clusters (correct): %d\n", total_clusters_correct);
    
    if (total_clusters_buggy != total_clusters_correct) {
        TEST_FAIL("Cluster count differs: buggy=%d vs correct=%d", 
                  total_clusters_buggy, total_clusters_correct);
    } else {
        TEST_PASS("Cluster counts match (bug may not manifest with this boot sector)");
    }
}

int main() {
    TEST_SUITE_START("FAT12 Calculations");
    
    test_data_start_macro();
    test_data_start_sectors_calculation();
    
    TEST_SUITE_END("FAT12 Calculations");
}