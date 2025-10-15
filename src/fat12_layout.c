/**
 * @file fat12_layout.c
 * @brief FAT12 disk layout offset calculations
 *
 * This module provides functions to calculate byte offsets for different
 * sections of a FAT12 disk (root directory, data area, clusters).
 * All calculations are dynamic and based on Boot Sector values.
 */

#include "fat12.h"
#include <stddef.h>

/* ========== FAT12 Constants ========== */

/** @brief Directory entry size in bytes (FAT12 standard) */
#define DIR_ENTRY_SIZE 32

/** @brief First valid data cluster number (clusters 0-1 are reserved) */
#define FIRST_DATA_CLUSTER 2

/** @brief Maximum valid cluster number for FAT12 filesystem */
#define MAX_FAT12_CLUSTER 4084

/* ========== Implementation ========== */

size_t calculate_root_directory_offset(const BootSector *boot)
{
    /* Validate input */
    if (boot == NULL) {
        return 0;
    }

    /* Calculate FAT area size in sectors:
     * Each FAT table has sectors_per_fat sectors
     * Total FAT area = num_fats × sectors_per_fat
     */
    size_t fat_area_sectors = boot->fat_count * boot->sectors_per_fat;

    /* Root directory starts after:
     * - Reserved sectors (boot sector + any additional reserved)
     * - All FAT tables
     *
     * Formula: (reserved_sectors + fat_area_sectors) × bytes_per_sector
     */
    size_t root_dir_start_sector = boot->reserved_sectors + fat_area_sectors;
    size_t root_dir_offset = root_dir_start_sector * boot->bytes_per_sector;

    return root_dir_offset;
}

size_t calculate_root_directory_size(const BootSector *boot)
{
    /* Validate input */
    if (boot == NULL) {
        return 0;
    }

    /* Root directory size is fixed:
     * Each directory entry is exactly 32 bytes (DIR_ENTRY_SIZE)
     * Total size = number of entries × bytes per entry
     *
     * Formula: root_entries × 32
     */
    size_t root_dir_size = boot->root_entries * DIR_ENTRY_SIZE;

    return root_dir_size;
}

size_t calculate_data_area_offset(const BootSector *boot)
{
    /* Validate input */
    if (boot == NULL) {
        return 0;
    }

    /* Data area starts immediately after the root directory.
     * This is where file and directory data (clusters) are stored.
     * Cluster numbering starts at 2.
     *
     * Formula: root_directory_offset + root_directory_size
     */
    size_t root_dir_offset = calculate_root_directory_offset(boot);
    size_t root_dir_size = calculate_root_directory_size(boot);
    size_t data_area_offset = root_dir_offset + root_dir_size;

    return data_area_offset;
}

size_t cluster_to_offset(uint16_t cluster, const BootSector *boot)
{
    /* Validate input */
    if (boot == NULL) {
        return 0;
    }

    /* Validate cluster number:
     * - Clusters 0 and 1 are reserved in FAT12
     * - Valid data clusters: 2 to 4084 (0xFF4)
     * - Cluster 4085+ (0xFF5+) are out of FAT12 range
     */
    if (cluster < FIRST_DATA_CLUSTER) {
        return 0;  /* Clusters 0 and 1 are reserved */
    }

    if (cluster > MAX_FAT12_CLUSTER) {
        return 0;  /* Out of FAT12 range */
    }

    /* Calculate cluster offset:
     * - Data area starts after root directory
     * - Cluster 2 is at offset 0 of data area (hence "cluster - 2")
     * - Each cluster occupies sectors_per_cluster × bytes_per_sector bytes
     *
     * Formula: data_area_offset + (cluster - 2) × cluster_size
     */
    size_t data_area_offset = calculate_data_area_offset(boot);
    size_t cluster_size = CLUSTER_SIZE(boot);
    size_t cluster_offset = data_area_offset + (cluster - FIRST_DATA_CLUSTER) * cluster_size;

    return cluster_offset;
}
