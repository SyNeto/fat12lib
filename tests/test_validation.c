#include "test_common.h"
#include "../include/fat12.h"
#include <string.h>
#include <limits.h>

FILE* create_temp_boot_sector(uint16_t bytes_per_sector, uint8_t sectors_per_cluster, 
                             uint16_t reserved_sectors, uint8_t fat_count, 
                             uint16_t root_entries, uint16_t total_sectors, 
                             uint16_t sectors_per_fat) {
    FILE* temp_file = tmpfile();
    if (!temp_file) return NULL;
    
    uint8_t boot_sector[512];
    memset(boot_sector, 0, 512);
    
    // FAT12 boot sector signature
    boot_sector[0] = 0xEB; boot_sector[1] = 0x3C; boot_sector[2] = 0x90;
    memcpy(&boot_sector[3], "MSDOS5.0", 8);
    
    // Fill BPB fields at correct offsets
    *(uint16_t*)(boot_sector + 0x0B) = bytes_per_sector;
    *(uint8_t*)(boot_sector + 0x0D) = sectors_per_cluster;
    *(uint16_t*)(boot_sector + 0x0E) = reserved_sectors;
    *(uint8_t*)(boot_sector + 0x10) = fat_count;
    *(uint16_t*)(boot_sector + 0x11) = root_entries;
    *(uint16_t*)(boot_sector + 0x13) = total_sectors;
    *(uint8_t*)(boot_sector + 0x15) = 0xF0; // media descriptor
    *(uint16_t*)(boot_sector + 0x16) = sectors_per_fat;
    
    fwrite(boot_sector, 512, 1, temp_file);
    return temp_file;
}

void test_boot_sector_validation() {
    printf("\n--- Testing read_boot_sector validation ---\n");
    
    BootSector boot;
    FILE* temp_file;
    int result;
    
    // Test 1: Valid boot sector
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(0, result, "Valid boot sector should be accepted");
    fclose(temp_file);
    
    // Test 2: Invalid bytes_per_sector
    temp_file = create_temp_boot_sector(256, 1, 1, 2, 224, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-3, result, "Invalid bytes_per_sector should be rejected");
    fclose(temp_file);
    
    // Test 3: Zero sectors_per_cluster
    temp_file = create_temp_boot_sector(512, 0, 1, 2, 224, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-4, result, "Zero sectors_per_cluster should be rejected");
    fclose(temp_file);
    
    // Test 4: Non-power-of-2 sectors_per_cluster
    temp_file = create_temp_boot_sector(512, 3, 1, 2, 224, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-11, result, "Non-power-of-2 sectors_per_cluster should be rejected");
    fclose(temp_file);
    
    // Test 5: Zero FAT count
    temp_file = create_temp_boot_sector(512, 1, 1, 0, 224, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-5, result, "Zero FAT count should be rejected");
    fclose(temp_file);
    
    // Test 6: Too many FATs
    temp_file = create_temp_boot_sector(512, 1, 1, 10, 224, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-5, result, "Too many FATs should be rejected");
    fclose(temp_file);
    
    // Test 7: Zero root entries
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 0, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-6, result, "Zero root entries should be rejected");
    fclose(temp_file);
    
    // Test 8: Zero sectors_per_fat
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 2880, 0);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-8, result, "Zero sectors_per_fat should be rejected");
    fclose(temp_file);
    
    // Test 9: Zero total_sectors
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 0, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-7, result, "Zero total_sectors should be rejected");
    fclose(temp_file);
    
    // Test 10: Too many reserved sectors
    temp_file = create_temp_boot_sector(512, 1, 1000, 2, 224, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-10, result, "Too many reserved sectors should be rejected");
    fclose(temp_file);
    
    // Test 11: Filesystem structure doesn't fit
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 10, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-12, result, "Insufficient total_sectors should be rejected");
    fclose(temp_file);
    
    // Test 12: Too many root entries
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 2000, 2880, 9);
    result = read_boot_sector(temp_file, &boot);
    TEST_ASSERT_EQ(-13, result, "Too many root entries should be rejected");
    fclose(temp_file);
}

void test_load_fat_table_size_validation() {
    printf("\n--- Testing load_fat_table size validation ---\n");
    
    BootSector boot;
    FILE* temp_file;
    uint8_t* fat_table;
    
    // Test 1: Normal FAT size (should work)
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 9);
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 2880, 9);
    
    // Add some dummy FAT data
    uint8_t dummy_fat[4608] = {0};
    fseek(temp_file, FAT_START((&boot)), SEEK_SET);
    fwrite(dummy_fat, sizeof(dummy_fat), 1, temp_file);
    
    fat_table = load_fat_table(temp_file, &boot);
    TEST_ASSERT(fat_table != NULL, "Normal FAT size should be accepted");
    if (fat_table) free_fat_table(fat_table);
    fclose(temp_file);
    
    // Test 2: Zero FAT size (should fail)
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 0);
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 2880, 0);
    fat_table = load_fat_table(temp_file, &boot);
    TEST_ASSERT(fat_table == NULL, "Zero FAT size should be rejected");
    fclose(temp_file);
    
    // Test 3: Huge FAT size (should fail)
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 200);
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 2880, 200);
    fat_table = load_fat_table(temp_file, &boot);
    TEST_ASSERT(fat_table == NULL, "Huge FAT size should be rejected");
    fclose(temp_file);
}

void test_analyze_fat_validation() {
    printf("\n--- Testing analyze_fat validation ---\n");
    
    BootSector boot;
    DiskInfo info;
    FILE* temp_file;
    int result;
    
    // Test 1: Normal case (should work)
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 2880, 9);
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 2880, 9);
    
    // Add dummy FAT data
    uint8_t dummy_fat[4608] = {0};
    fseek(temp_file, FAT_START((&boot)), SEEK_SET);
    fwrite(dummy_fat, sizeof(dummy_fat), 1, temp_file);
    
    result = analyze_fat(temp_file, &boot, &info);
    TEST_ASSERT_EQ(0, result, "Normal FAT analysis should succeed");
    fclose(temp_file);
    
    // Test 2: Too many clusters for FAT12
    create_mock_boot_sector(&boot, 512, 1, 1, 2, 224, 65535, 9);
    temp_file = create_temp_boot_sector(512, 1, 1, 2, 224, 65535, 9);
    
    fseek(temp_file, FAT_START((&boot)), SEEK_SET);
    fwrite(dummy_fat, sizeof(dummy_fat), 1, temp_file);
    
    result = analyze_fat(temp_file, &boot, &info);
    TEST_ASSERT_EQ(-1, result, "Too many clusters should be rejected");
    fclose(temp_file);
}

int main() {
    TEST_SUITE_START("Input Validation");
    
    test_boot_sector_validation();
    test_load_fat_table_size_validation();
    test_analyze_fat_validation();
    
    TEST_SUITE_END("Input Validation");
}