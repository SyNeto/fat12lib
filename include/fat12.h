#ifndef FAT12_H
#define FAT12_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief FAT12 Filesystem Layout Calculation Macros
 * 
 * These macros calculate byte offsets for different sections of a FAT12 disk image.
 * The layout is: Boot Sector -> Reserved Sectors -> FAT Tables -> Root Directory -> Data Area
 */

/** @brief Starting offset of the boot sector (always at beginning of disk) */
#define BOOT_SECTOR_OFFSET 0

/** @brief Calculate starting byte offset of the first FAT table */
#define FAT_START(boot) (BOOT_SECTOR_OFFSET + (boot->reserved_sectors * boot->bytes_per_sector))

/** @brief Calculate starting byte offset of the root directory */
#define ROOT_DIR_START(boot) (FAT_START(boot) + (boot->fat_count * boot->sectors_per_fat * boot->bytes_per_sector))

/** 
 * @brief Calculate starting byte offset of the data area (clusters 2+)
 * 
 * Uses ceiling division: (root_entries * 32 + sector_size - 1) / sector_size
 * to ensure root directory occupies complete sectors.
 */
#define DATA_START(boot) (ROOT_DIR_START(boot) + ((boot->root_entries * 32 + boot->bytes_per_sector - 1) / boot->bytes_per_sector) * boot->bytes_per_sector)

/**
 * @brief Represents the boot sector of a FAT12 file system
 * 
 * Note: This struct contains only the fields we need, not the entire boot sector.
 * Fields are read from specific offsets in the 512-byte boot sector on disk.
 */
typedef struct {
    uint16_t bytes_per_sector;     // Offset 0x0B in boot sector
    uint8_t sectors_per_cluster;   // Offset 0x0D in boot sector  
    uint16_t reserved_sectors;     // Offset 0x0E in boot sector
    uint8_t fat_count;             // Offset 0x10 in boot sector
    uint16_t root_entries;         // Offset 0x11 in boot sector
    uint16_t total_sectors;        // Offset 0x13 in boot sector
    uint16_t sectors_per_fat;      // Offset 0x16 in boot sector
} __attribute__((packed)) BootSector;

/**
 * @brief Contains information about the disk image
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
 * @brief Analyzes the FAT and gathers information about the disk image.
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

/**
 * @brief Loads the FAT table into memory for editing purposes.
 * 
 * @param img Pointer to the file representing the disk image.
 * @param boot Pointer to the BootSector structure containing the boot sector data.
 * @return Pointer to the FAT table in memory, or NULL on failure.
 */
uint8_t* load_fat_table(FILE *img, const BootSector *boot);

/**
 * @brief Frees the memory allocated for the FAT table.
 * 
 * @param fat_table Pointer to the FAT table to free.
 */
void free_fat_table(uint8_t* fat_table);

/**
 * @brief Extracts a 12-bit FAT12 entry from the FAT table.
 * 
 * @param fat_table Pointer to the FAT table loaded in memory.
 * @param fat_size_bytes Size of the FAT table in bytes.
 * @param cluster Cluster number (starting from 0).
 * @return 12-bit FAT entry value (0x000 to 0xFFF), or 0xFFFF on bounds error.
 */
uint16_t get_fat12_entry(const uint8_t* fat_table, int fat_size_bytes, int cluster);

/**
 * @brief Sets a 12-bit FAT12 entry in the FAT table.
 * 
 * @param fat_table Pointer to the FAT table loaded in memory.
 * @param fat_size_bytes Size of the FAT table in bytes.
 * @param cluster Cluster number (starting from 0).
 * @param value 12-bit value to set (0x000 to 0xFFF).
 * @return 0 on success, -1 on bounds error.
 */
int set_fat12_entry(uint8_t* fat_table, int fat_size_bytes, int cluster, uint16_t value);

#endif // FAT12_H
