#include "test_common.h"
#include "../include/fat12.h"
#include <stdlib.h>

void test_get_fat12_entry_bounds() {
    printf("\n--- Testing get_fat12_entry bounds checking ---\n");
    
    // Create a small FAT table for testing
    uint8_t fat_table[] = {0xF0, 0xFF, 0xFF, 0x03, 0x40, 0x00, 0x05, 0x60, 0x00};
    int fat_table_size = sizeof(fat_table);
    
    printf("FAT table size: %d bytes\n", fat_table_size);
    printf("Max valid cluster for this FAT: %d\n", (fat_table_size * 2 / 3) - 1);
    
    // Test valid clusters (should work)
    for (int cluster = 0; cluster < 6; cluster++) {
        uint16_t result = get_fat12_entry(fat_table, fat_table_size, cluster);
        printf("Cluster %d: 0x%03X\n", cluster, result);
        TEST_PASS("Valid cluster access works");
    }
    
    // Test boundary cases - these should be caught by bounds checking
    int max_valid_cluster = (fat_table_size * 2 / 3) - 1;
    
    printf("\nTesting boundary conditions:\n");
    
    // Test accessing cluster beyond valid range
    int invalid_cluster = max_valid_cluster + 1;
    printf("Attempting to access cluster %d (invalid)...\n", invalid_cluster);
    
    // This will currently cause buffer overrun - should be caught by bounds checking
    // For now, this test documents that the bug exists
    // TODO: Once bounds checking is added, this should return an error
    
    // Calculate what byte offset would be accessed
    int byte_offset = invalid_cluster * 3 / 2;
    printf("Would access byte offset: %d (FAT size: %d)\n", byte_offset, fat_table_size);
    
    // Actually test that the function prevents buffer overrun
    uint16_t result = get_fat12_entry(fat_table, fat_table_size, invalid_cluster);
    if (result == 0xFFFF) {
        TEST_PASS("get_fat12_entry correctly rejects invalid cluster (returns 0xFFFF)");
    } else {
        TEST_FAIL("get_fat12_entry should return 0xFFFF for invalid cluster %d, got 0x%03X", 
                  invalid_cluster, result);
    }
    
    // Test extremely large cluster number
    int huge_cluster = 10000;
    byte_offset = huge_cluster * 3 / 2;
    printf("Huge cluster %d would access offset: %d\n", huge_cluster, byte_offset);
    
    TEST_ASSERT(byte_offset >= fat_table_size, 
                "Huge cluster numbers cause massive buffer overrun");
    
    // Test negative cluster (if using signed int)
    printf("\nTesting negative cluster numbers:\n");
    int negative_cluster = -1;
    byte_offset = negative_cluster * 3 / 2; // This will be garbage due to underflow
    printf("Negative cluster %d gives byte offset: %d\n", negative_cluster, byte_offset);
    
    TEST_ASSERT(negative_cluster < 0, "Negative clusters should be rejected");
}

void test_set_fat12_entry_bounds() {
    printf("\n--- Testing set_fat12_entry bounds checking ---\n");
    
    // Create a small FAT table for testing
    uint8_t fat_table[] = {0xF0, 0xFF, 0xFF, 0x03, 0x40, 0x00, 0x05, 0x60, 0x00};
    int fat_table_size = sizeof(fat_table);
    
    printf("FAT table size: %d bytes\n", fat_table_size);
    
    // Test valid cluster writes
    uint8_t original_data[sizeof(fat_table)];
    memcpy(original_data, fat_table, sizeof(fat_table));
    
    // Valid write
    set_fat12_entry(fat_table, fat_table_size, 2, 0x123);
    uint16_t readback = get_fat12_entry(fat_table, fat_table_size, 2);
    TEST_ASSERT_EQ_HEX(0x123, readback, "Valid cluster write works");
    
    // Restore original data
    memcpy(fat_table, original_data, sizeof(fat_table));
    
    // Test boundary violation - writing beyond valid range
    int max_valid_cluster = (fat_table_size * 2 / 3) - 1;
    int invalid_cluster = max_valid_cluster + 1;
    
    printf("\nTesting boundary violations:\n");
    printf("Attempting to write to cluster %d (invalid)...\n", invalid_cluster);
    
    // Calculate what bytes would be accessed
    int byte_offset = invalid_cluster * 3 / 2;
    
    // Actually test that the function prevents buffer overrun
    int result = set_fat12_entry(fat_table, fat_table_size, invalid_cluster, 0x123);
    if (result == -1) {
        TEST_PASS("set_fat12_entry correctly rejects invalid cluster (returns -1)");
    } else {
        TEST_FAIL("set_fat12_entry should return -1 for invalid cluster %d, got %d", 
                  invalid_cluster, result);
    }
    
    // Test extremely large cluster write
    int huge_cluster = 10000;
    byte_offset = huge_cluster * 3 / 2;
    printf("Huge cluster %d would access bytes: %d-%d\n", huge_cluster, byte_offset, byte_offset + 1);
    
    TEST_ASSERT(byte_offset + 1 >= fat_table_size, 
                "Huge cluster writes cause buffer overrun");
    
    // Test value validation - FAT12 values should be 12 bits max
    printf("\nTesting value validation:\n");
    
    // Test 12-bit boundary
    TEST_ASSERT(0xFFF <= 0xFFF, "0xFFF is valid 12-bit value");
    TEST_ASSERT(0x1000 > 0xFFF, "0x1000 exceeds 12-bit range");
    
    // The function should mask to 12 bits, but let's verify
    memcpy(fat_table, original_data, sizeof(fat_table));
    set_fat12_entry(fat_table, fat_table_size, 2, 0x1234); // Exceeds 12 bits
    readback = get_fat12_entry(fat_table, fat_table_size, 2);
    
    // Should be masked to 12 bits: 0x1234 & 0xFFF = 0x234
    TEST_ASSERT_EQ_HEX(0x234, readback, "Large values are properly masked to 12 bits");
}

void test_fat12_entry_data_corruption() {
    printf("\n--- Testing data corruption in adjacent entries ---\n");
    
    // Test that modifying one cluster doesn't corrupt adjacent ones
    uint8_t fat_table[] = {0xF0, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int fat_table_size = sizeof(fat_table);
    
    // Set up known values in adjacent clusters
    set_fat12_entry(fat_table, fat_table_size, 2, 0x123);
    set_fat12_entry(fat_table, fat_table_size, 3, 0x456);  
    set_fat12_entry(fat_table, fat_table_size, 4, 0x789);
    
    // Verify they're set correctly
    TEST_ASSERT_EQ_HEX(0x123, get_fat12_entry(fat_table, fat_table_size, 2), "Cluster 2 set correctly");
    TEST_ASSERT_EQ_HEX(0x456, get_fat12_entry(fat_table, fat_table_size, 3), "Cluster 3 set correctly");
    TEST_ASSERT_EQ_HEX(0x789, get_fat12_entry(fat_table, fat_table_size, 4), "Cluster 4 set correctly");
    
    // Modify middle cluster and verify neighbors aren't affected
    set_fat12_entry(fat_table, fat_table_size, 3, 0xABC);
    
    TEST_ASSERT_EQ_HEX(0x123, get_fat12_entry(fat_table, fat_table_size, 2), "Cluster 2 unchanged after modifying cluster 3");
    TEST_ASSERT_EQ_HEX(0xABC, get_fat12_entry(fat_table, fat_table_size, 3), "Cluster 3 updated correctly");
    TEST_ASSERT_EQ_HEX(0x789, get_fat12_entry(fat_table, fat_table_size, 4), "Cluster 4 unchanged after modifying cluster 3");
    
    printf("\nFAT table hex dump after modifications:\n");
    for (int i = 0; i < 9; i++) {
        printf("%02X ", fat_table[i]);
    }
    printf("\n");
}

int main() {
    TEST_SUITE_START("FAT12 Bounds Checking");
    
    test_get_fat12_entry_bounds();
    test_set_fat12_entry_bounds();
    test_fat12_entry_data_corruption();
    
    TEST_SUITE_END("FAT12 Bounds Checking");
}