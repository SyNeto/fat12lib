#include "test_common.h"
#include "../include/fat12.h"
#include <string.h>
#include <stddef.h>

void test_fixed_boot_sector_reading() {
    printf("\n--- Testing fixed boot sector reading ---\n");
    
    // Create a simulated FAT12 boot sector
    uint8_t fake_boot_sector[512];
    memset(fake_boot_sector, 0, 512);
    
    // Fill in the FAT12 boot sector fields at correct offsets
    fake_boot_sector[0] = 0xEB; fake_boot_sector[1] = 0x3C; fake_boot_sector[2] = 0x90; // Jump
    memcpy(&fake_boot_sector[3], "MSDOS5.0", 8); // OEM name
    
    // FAT12 BPB fields at correct offsets
    *(uint16_t*)(fake_boot_sector + 0x0B) = 512;    // bytes_per_sector
    *(uint8_t*)(fake_boot_sector + 0x0D) = 1;       // sectors_per_cluster
    *(uint16_t*)(fake_boot_sector + 0x0E) = 1;      // reserved_sectors  
    *(uint8_t*)(fake_boot_sector + 0x10) = 2;       // fat_count
    *(uint16_t*)(fake_boot_sector + 0x11) = 224;    // root_entries
    *(uint16_t*)(fake_boot_sector + 0x13) = 2880;   // total_sectors
    *(uint8_t*)(fake_boot_sector + 0x15) = 0xF0;    // media_descriptor
    *(uint16_t*)(fake_boot_sector + 0x16) = 9;      // sectors_per_fat
    
    // Create a temporary file with this boot sector
    FILE* temp_file = tmpfile();
    if (!temp_file) {
        TEST_FAIL("Could not create temporary file");
        return;
    }
    
    // Write the fake boot sector
    fwrite(fake_boot_sector, 512, 1, temp_file);
    
    // Test the fixed reading
    BootSector boot;
    int result = read_boot_sector(temp_file, &boot);
    
    printf("read_boot_sector result: %d\n", result);
    printf("Read values:\n");
    printf("  bytes_per_sector: %u\n", boot.bytes_per_sector);
    printf("  sectors_per_cluster: %u\n", boot.sectors_per_cluster);
    printf("  reserved_sectors: %u\n", boot.reserved_sectors);
    printf("  fat_count: %u\n", boot.fat_count);
    printf("  root_entries: %u\n", boot.root_entries);
    printf("  total_sectors: %u\n", boot.total_sectors);
    printf("  sectors_per_fat: %u\n", boot.sectors_per_fat);
    
    // Test all values
    TEST_ASSERT_EQ(0, result, "read_boot_sector should succeed");
    TEST_ASSERT_EQ(512, boot.bytes_per_sector, "bytes_per_sector correct");
    TEST_ASSERT_EQ(1, boot.sectors_per_cluster, "sectors_per_cluster correct");
    TEST_ASSERT_EQ(1, boot.reserved_sectors, "reserved_sectors correct");
    TEST_ASSERT_EQ(2, boot.fat_count, "fat_count correct");
    TEST_ASSERT_EQ(224, boot.root_entries, "root_entries correct");
    TEST_ASSERT_EQ(2880, boot.total_sectors, "total_sectors correct");
    TEST_ASSERT_EQ(9, boot.sectors_per_fat, "sectors_per_fat correct");
    
    fclose(temp_file);
}

void test_boot_sector_validation() {
    printf("\n--- Testing enhanced boot sector validation ---\n");
    
    uint8_t bad_boot_sector[512];
    memset(bad_boot_sector, 0, 512);
    
    // Test various invalid configurations
    FILE* temp_file;
    BootSector boot;
    int result;
    
    // Test 1: Invalid bytes_per_sector
    memset(bad_boot_sector, 0, 512);
    *(uint16_t*)(bad_boot_sector + 0x0B) = 256; // Invalid sector size
    *(uint8_t*)(bad_boot_sector + 0x0D) = 1;
    *(uint16_t*)(bad_boot_sector + 0x0E) = 1;
    *(uint8_t*)(bad_boot_sector + 0x10) = 2;
    *(uint16_t*)(bad_boot_sector + 0x11) = 224;
    *(uint16_t*)(bad_boot_sector + 0x13) = 2880;
    *(uint16_t*)(bad_boot_sector + 0x16) = 9;
    
    temp_file = tmpfile();
    fwrite(bad_boot_sector, 512, 1, temp_file);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-3, result, "Should reject invalid bytes_per_sector");
    fclose(temp_file);
    
    // Test 2: Zero sectors_per_cluster
    memset(bad_boot_sector, 0, 512);
    *(uint16_t*)(bad_boot_sector + 0x0B) = 512;
    *(uint8_t*)(bad_boot_sector + 0x0D) = 0; // Invalid
    *(uint16_t*)(bad_boot_sector + 0x0E) = 1;
    *(uint8_t*)(bad_boot_sector + 0x10) = 2;
    *(uint16_t*)(bad_boot_sector + 0x11) = 224;
    *(uint16_t*)(bad_boot_sector + 0x13) = 2880;
    *(uint16_t*)(bad_boot_sector + 0x16) = 9;
    
    temp_file = tmpfile();
    fwrite(bad_boot_sector, 512, 1, temp_file);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-4, result, "Should reject zero sectors_per_cluster");
    fclose(temp_file);
    
    // Test 3: Invalid FAT count
    memset(bad_boot_sector, 0, 512);
    *(uint16_t*)(bad_boot_sector + 0x0B) = 512;
    *(uint8_t*)(bad_boot_sector + 0x0D) = 1;
    *(uint16_t*)(bad_boot_sector + 0x0E) = 1;
    *(uint8_t*)(bad_boot_sector + 0x10) = 0; // Invalid
    *(uint16_t*)(bad_boot_sector + 0x11) = 224;
    *(uint16_t*)(bad_boot_sector + 0x13) = 2880;
    *(uint16_t*)(bad_boot_sector + 0x16) = 9;
    
    temp_file = tmpfile();
    fwrite(bad_boot_sector, 512, 1, temp_file);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-5, result, "Should reject zero FAT count");
    fclose(temp_file);
    
    printf("Enhanced validation working correctly!\n");
}

void test_struct_size_after_fix() {
    printf("\n--- Testing struct size after __attribute__((packed)) ---\n");
    
    printf("BootSector struct size: %zu bytes\n", sizeof(BootSector));
    
    // Calculate expected size
    size_t expected_size = 2 + 1 + 2 + 1 + 2 + 2 + 2; // 12 bytes
    
    TEST_ASSERT_EQ(expected_size, sizeof(BootSector), "Struct should be packed (no padding)");
    
    printf("Struct is now properly packed!\n");
}

int main() {
    TEST_SUITE_START("Fixed Boot Sector Reading");
    
    test_struct_size_after_fix();
    test_fixed_boot_sector_reading();
    test_boot_sector_validation();
    
    TEST_SUITE_END("Fixed Boot Sector Reading");
}