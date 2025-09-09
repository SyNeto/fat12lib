#include "fat12.h"
#include <stdlib.h>
#include <string.h>

#define BOOT_SECTOR_OFFSET 0
#define FAT_START(boot) (BOOT_SECTOR_OFFSET + (boot->reserved_sectors * boot->bytes_per_sector))
#define ROOT_DIR_START(boot) (FAT_START(boot) + (boot->fat_count * boot->sectors_per_fat * boot->bytes_per_sector))
#define DATA_START(boot) (ROOT_DIR_START(boot) + (boot->root_entries * 32 / boot-> bytes_per_sector))


int read_boot_sector(FILE *img, BootSector *boot)
{
    fseek(img, BOOT_SECTOR_OFFSET, SEEK_SET);
    fread(boot, sizeof(BootSector), 1, img);

    if (boot->bytes_per_sector != 512 || boot->sectors_per_cluster == 0) {
        return -1; // Invalid boot sector
    }
    return 0;
}

/**
 * @brief Extracts a 12-bit FAT12 entry from the FAT table
 * 
 * FAT12 entries are packed: 2 entries occupy 3 bytes
 * Example: bytes [AB][CD][EF] contain entries [BAD][EFC] (little-endian)
 * 
 * @param fat_table Pointer to the FAT table loaded in memory
 * @param cluster Cluster number (starting from 0)
 * @return 12-bit FAT entry value (0x000 to 0xFFF)
 */
uint16_t get_fat12_entry(const uint8_t* fat_table, int cluster)
{
    // Each entry occupies 1.5 bytes (12 bits)
    int byte_offset = cluster * 3 / 2;
    
    if (cluster % 2 == 0) {
        // Even cluster: take lower 12 bits
        // Complete low byte + low nibble of high byte
        return (fat_table[byte_offset + 1] & 0x0F) << 8 | fat_table[byte_offset];
    } else {
        // Odd cluster: take upper 12 bits  
        // Complete high byte + high nibble of low byte
        return fat_table[byte_offset + 1] << 4 | (fat_table[byte_offset] >> 4);
    }
}

// API for editable FAT table - uses malloc with goto cleanup
uint8_t* load_fat_table(FILE *img, const BootSector *boot)
{
    uint8_t* fat_table = NULL;
    int fat_size_bytes = boot->sectors_per_fat * boot->bytes_per_sector;
    
    fat_table = malloc(fat_size_bytes);
    if (!fat_table) goto error;
    
    if (fseek(img, FAT_START(boot), SEEK_SET) != 0) goto error;
    if (fread(fat_table, fat_size_bytes, 1, img) != 1) goto error;
    
    return fat_table;  // SUCCESS
    
error:
    free(fat_table);   // Single cleanup point
    return NULL;
}

void free_fat_table(uint8_t* fat_table)
{
    free(fat_table);
}

void set_fat12_entry(uint8_t* fat_table, int cluster, uint16_t value)
{
    // Each entry occupies 1.5 bytes (12 bits)
    int byte_offset = cluster * 3 / 2;
    
    // Mask to 12 bits
    value &= 0xFFF;
    
    if (cluster % 2 == 0) {
        // Even cluster: set lower 12 bits
        fat_table[byte_offset] = value & 0xFF;                    // Low byte
        fat_table[byte_offset + 1] = (fat_table[byte_offset + 1] & 0xF0) | ((value >> 8) & 0x0F); // High nibble
    } else {
        // Odd cluster: set upper 12 bits
        fat_table[byte_offset] = (fat_table[byte_offset] & 0x0F) | ((value & 0x0F) << 4);  // Low nibble
        fat_table[byte_offset + 1] = (value >> 4) & 0xFF;        // High byte
    }
}

// Internal analysis function - uses VLA for efficiency
int analyze_fat(FILE *img, const BootSector *boot, DiskInfo *info)
{
    // Calculate data area start in sectors (fixed precedence)
    int data_start_sectors = DATA_START(boot) / boot->bytes_per_sector;
    int total_data_sectors = boot->total_sectors - data_start_sectors;
    int total_clusters = total_data_sectors / boot->sectors_per_cluster;
    
    // Validate FAT12 range (max 4084 clusters)
    if (total_clusters > 4084) {
        return -1; // Invalid FAT12 - too many clusters
    }
    
    // Use VLA for temporary analysis - automatic cleanup
    int fat_size_bytes = boot->sectors_per_fat * boot->bytes_per_sector;
    uint8_t fat_table[fat_size_bytes];  // VLA - stack allocation
    
    // Read FAT table
    if (fseek(img, FAT_START(boot), SEEK_SET) != 0) {
        return -3; // Seek failed - no cleanup needed
    }
    
    if (fread(fat_table, fat_size_bytes, 1, img) != 1) {
        return -4; // Read failed - no cleanup needed
    }
    
    // Initialize counters
    info->total_clusters = total_clusters;
    info->used_clusters = 0;
    info->free_clusters = 0;
    
    // Iterate through valid clusters (start at 2, clusters 0-1 are reserved)
    for (int cluster = 2; cluster < total_clusters + 2; cluster++) {
        uint16_t fat_entry = get_fat12_entry(fat_table, cluster);
        
        // Classify cluster based on FAT12 standard values
        if (fat_entry == 0x000) {
            info->free_clusters++;           // Free cluster
        } else if (fat_entry == 0x001) {
            // Reserved cluster - don't count
            continue;
        } else if (fat_entry >= 0x002 && fat_entry <= 0xFF6) {
            info->used_clusters++;           // Part of file chain
        } else if (fat_entry == 0xFF7) {
            // Bad cluster - don't count as free or used
            continue;
        } else if (fat_entry >= 0xFF8 && fat_entry <= 0xFFF) {
            info->used_clusters++;           // End of file marker
        }
    }
    
    return 0;  // VLA automatically cleaned up
}

void print_disk_info(const DiskInfo *info)
{
    printf("Sector size: %d bytes\n", info->boot_sector.bytes_per_sector);
    printf("Sectors per cluster: %d\n", info->boot_sector.sectors_per_cluster);
    printf("Number of FATs: %d\n", info->boot_sector.fat_count);
    printf("Root directory entries: %d\n", info->boot_sector.root_entries);
    printf("Total clusters: %d\n", info->total_clusters);
    printf("Used clusters: %d\n", info->used_clusters);
    printf("Free clusters: %d\n", info->free_clusters);
}
