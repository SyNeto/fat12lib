/**
 * @file test_layout.c
 * @brief Unit tests for FAT12 layout offset calculations
 *
 * This test suite validates dynamic offset calculations for different
 * sections of a FAT12 disk based on boot sector values.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "fat12.h"

/* Test counter for progress reporting */
static int tests_run = 0;
static int tests_passed = 0;

/* Helper macro for test execution */
#define RUN_TEST(test_func) do { \
    printf("Running: %s...", #test_func); \
    tests_run++; \
    test_func(); \
    tests_passed++; \
    printf(" PASSED\n"); \
} while(0)

/* ========== Test Functions ========== */

/* Tests for calculate_root_directory_offset() */

void test_root_dir_offset_1440kb_floppy(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Formula: (reserved_sectors + num_fats × sectors_per_fat) × bytes_per_sector
     * Expected: (1 + 2×9) × 512 = 19 × 512 = 9728 bytes
     */
    size_t offset = calculate_root_directory_offset(&boot);
    assert(offset == 9728);
}

void test_root_dir_offset_720kb_floppy(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 2,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 3,
        .root_entries = 112
    };

    /* Formula: (1 + 2×3) × 512 = 7 × 512 = 3584 bytes */
    size_t offset = calculate_root_directory_offset(&boot);
    assert(offset == 3584);
}

void test_root_dir_offset_custom_config(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 4,
        .reserved_sectors = 4,
        .fat_count = 2,
        .sectors_per_fat = 16,
        .root_entries = 512
    };

    /* Formula: (4 + 2×16) × 512 = 36 × 512 = 18432 bytes */
    size_t offset = calculate_root_directory_offset(&boot);
    assert(offset == 18432);
}

void test_root_dir_offset_null_pointer(void)
{
    /* NULL pointer should return 0 */
    size_t offset = calculate_root_directory_offset(NULL);
    assert(offset == 0);
}

/* Tests for calculate_root_directory_size() */

void test_root_dir_size_1440kb_floppy(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Formula: root_entries × 32
     * Expected: 224 × 32 = 7168 bytes
     */
    size_t size = calculate_root_directory_size(&boot);
    assert(size == 7168);
}

void test_root_dir_size_720kb_floppy(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 2,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 3,
        .root_entries = 112
    };

    /* Formula: 112 × 32 = 3584 bytes */
    size_t size = calculate_root_directory_size(&boot);
    assert(size == 3584);
}

void test_root_dir_size_custom_config(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 4,
        .reserved_sectors = 4,
        .fat_count = 2,
        .sectors_per_fat = 16,
        .root_entries = 512
    };

    /* Formula: 512 × 32 = 16384 bytes */
    size_t size = calculate_root_directory_size(&boot);
    assert(size == 16384);
}

void test_root_dir_size_null_pointer(void)
{
    /* NULL pointer should return 0 */
    size_t size = calculate_root_directory_size(NULL);
    assert(size == 0);
}

/* Tests for calculate_data_area_offset() */

void test_data_area_offset_1440kb_floppy(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Formula: root_directory_offset + root_directory_size
     * Root dir offset: 9728
     * Root dir size: 7168
     * Expected: 9728 + 7168 = 16896 bytes
     */
    size_t offset = calculate_data_area_offset(&boot);
    assert(offset == 16896);
}

void test_data_area_offset_720kb_floppy(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 2,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 3,
        .root_entries = 112
    };

    /* Root dir offset: 3584
     * Root dir size: 3584
     * Expected: 3584 + 3584 = 7168 bytes
     */
    size_t offset = calculate_data_area_offset(&boot);
    assert(offset == 7168);
}

void test_data_area_offset_custom_config(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 4,
        .reserved_sectors = 4,
        .fat_count = 2,
        .sectors_per_fat = 16,
        .root_entries = 512
    };

    /* Root dir offset: 18432
     * Root dir size: 16384
     * Expected: 18432 + 16384 = 34816 bytes
     */
    size_t offset = calculate_data_area_offset(&boot);
    assert(offset == 34816);
}

void test_data_area_offset_null_pointer(void)
{
    /* NULL pointer should return 0 */
    size_t offset = calculate_data_area_offset(NULL);
    assert(offset == 0);
}

/* Tests for cluster_to_offset() */

void test_cluster_to_offset_first_cluster(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Cluster 2 is the first data cluster
     * Data area starts at 16896
     * Formula: 16896 + (2-2)×512 = 16896 bytes
     */
    size_t offset = cluster_to_offset(2, &boot);
    assert(offset == 16896);
}

void test_cluster_to_offset_second_cluster(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Cluster 3
     * Formula: 16896 + (3-2)×512 = 16896 + 512 = 17408 bytes
     */
    size_t offset = cluster_to_offset(3, &boot);
    assert(offset == 17408);
}

void test_cluster_to_offset_fifth_cluster(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Cluster 5
     * Formula: 16896 + (5-2)×512 = 16896 + 1536 = 18432 bytes
     */
    size_t offset = cluster_to_offset(5, &boot);
    assert(offset == 18432);
}

void test_cluster_to_offset_multiple_sectors_per_cluster(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 4,
        .reserved_sectors = 4,
        .fat_count = 2,
        .sectors_per_fat = 16,
        .root_entries = 512
    };

    /* Data area offset: 34816
     * Cluster size: 4 × 512 = 2048 bytes
     * Cluster 2: 34816 + (2-2)×2048 = 34816
     * Cluster 3: 34816 + (3-2)×2048 = 34816 + 2048 = 36864
     */
    size_t offset2 = cluster_to_offset(2, &boot);
    assert(offset2 == 34816);

    size_t offset3 = cluster_to_offset(3, &boot);
    assert(offset3 == 36864);
}

void test_cluster_to_offset_720kb_floppy(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 2,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 3,
        .root_entries = 112
    };

    /* Data area offset: 7168
     * Cluster size: 2 × 512 = 1024 bytes
     * Cluster 2: 7168 + (2-2)×1024 = 7168
     * Cluster 3: 7168 + (3-2)×1024 = 7168 + 1024 = 8192
     */
    size_t offset = cluster_to_offset(2, &boot);
    assert(offset == 7168);

    offset = cluster_to_offset(3, &boot);
    assert(offset == 8192);
}

void test_cluster_to_offset_invalid_cluster_zero(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Cluster 0 is reserved, should return 0 */
    size_t offset = cluster_to_offset(0, &boot);
    assert(offset == 0);
}

void test_cluster_to_offset_invalid_cluster_one(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Cluster 1 is reserved, should return 0 */
    size_t offset = cluster_to_offset(1, &boot);
    assert(offset == 0);
}

void test_cluster_to_offset_out_of_range(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Cluster 4085 and above are out of FAT12 range, should return 0 */
    size_t offset = cluster_to_offset(4085, &boot);
    assert(offset == 0);

    offset = cluster_to_offset(5000, &boot);
    assert(offset == 0);
}

void test_cluster_to_offset_max_valid_cluster(void)
{
    BootSector boot = {
        .bytes_per_sector = 512,
        .sectors_per_cluster = 1,
        .reserved_sectors = 1,
        .fat_count = 2,
        .sectors_per_fat = 9,
        .root_entries = 224
    };

    /* Cluster 4084 is the maximum valid cluster for FAT12
     * Formula: 16896 + (4084-2)×512 = 16896 + 2089984 = 2106880
     */
    size_t offset = cluster_to_offset(4084, &boot);
    assert(offset == 2106880);
}

void test_cluster_to_offset_null_pointer(void)
{
    /* NULL pointer should return 0 */
    size_t offset = cluster_to_offset(5, NULL);
    assert(offset == 0);
}

/* ========== Main Test Runner ========== */

int main(void)
{
    printf("\n");
    printf("=====================================\n");
    printf("  FAT12 Layout Offset Tests\n");
    printf("=====================================\n\n");

    /* Test calculate_root_directory_offset() */
    RUN_TEST(test_root_dir_offset_1440kb_floppy);
    RUN_TEST(test_root_dir_offset_720kb_floppy);
    RUN_TEST(test_root_dir_offset_custom_config);
    RUN_TEST(test_root_dir_offset_null_pointer);

    /* Test calculate_root_directory_size() */
    RUN_TEST(test_root_dir_size_1440kb_floppy);
    RUN_TEST(test_root_dir_size_720kb_floppy);
    RUN_TEST(test_root_dir_size_custom_config);
    RUN_TEST(test_root_dir_size_null_pointer);

    /* Test calculate_data_area_offset() */
    RUN_TEST(test_data_area_offset_1440kb_floppy);
    RUN_TEST(test_data_area_offset_720kb_floppy);
    RUN_TEST(test_data_area_offset_custom_config);
    RUN_TEST(test_data_area_offset_null_pointer);

    /* Test cluster_to_offset() */
    RUN_TEST(test_cluster_to_offset_first_cluster);
    RUN_TEST(test_cluster_to_offset_second_cluster);
    RUN_TEST(test_cluster_to_offset_fifth_cluster);
    RUN_TEST(test_cluster_to_offset_multiple_sectors_per_cluster);
    RUN_TEST(test_cluster_to_offset_720kb_floppy);
    RUN_TEST(test_cluster_to_offset_invalid_cluster_zero);
    RUN_TEST(test_cluster_to_offset_invalid_cluster_one);
    RUN_TEST(test_cluster_to_offset_out_of_range);
    RUN_TEST(test_cluster_to_offset_max_valid_cluster);
    RUN_TEST(test_cluster_to_offset_null_pointer);

    printf("\n");
    printf("=====================================\n");
    printf("  Test Results: %d/%d passed\n", tests_passed, tests_run);
    printf("=====================================\n\n");

    return (tests_run == tests_passed) ? 0 : 1;
}
