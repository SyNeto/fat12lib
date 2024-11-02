#ifndef FAT12_H
#define FAT12_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Represents the boot sector of a FAT12 file system
 */
typedef struct {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_entries;
    uint16_t total_sectors;
    uint16_t sectors_per_fat;
} BootSector;

/**
 * @brief Contains information acout the disk image
 */
typedef struct {
    BootSector boot_sector;
    int total_clusters;
    int used_clusters;
    int free_clusters;
} DiskInfo;

/**
 * @brief Reads the boot sector from the disk image.
 * 
 * @param img Pointer to the file representing the disk image.
 * @param boot Pointer to the BootSector structure to store the boot sector data.
 * @return int 0 if successful, non-zero on failure.
 */
int read_boot_sector(FILE *img, BootSector *boot);

/**
 * @brief Analyze the FAT and gathers information about the disk image.
 * 
 * @param img Pointer to the file representing the disk image.
 * @param boot Pointer to the BootSector structure containing the boot sector data.
 * @param info Pointer to the DiskInfo structure to store the disk information.
 * @return int 0 if successful, non-zero on failure.
 */
int analyze_fat(FILE *img, const BootSector *boot, DiskInfo *info);

/**
 * @brief Prints the disk information.
 * 
 * @param info Pointer to the DiskInfo structure containing the disk information.
 * @return void
 */
void print_disk_info(const DiskInfo *info);

#endif // FAT12_H
