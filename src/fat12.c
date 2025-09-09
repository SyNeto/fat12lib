#include "fat12.h"
#include <stdlib.h>
#include <string.h>

int read_boot_sector(FILE *img, BootSector *boot)
{
    uint8_t boot_sector[512];
    
    if (fseek(img, BOOT_SECTOR_OFFSET, SEEK_SET) != 0) {
        return -1;
    }
    
    if (fread(boot_sector, 512, 1, img) != 1) {
        return -2;
    }
    
    boot->bytes_per_sector = *(uint16_t*)(boot_sector + 0x0B);
    boot->sectors_per_cluster = *(uint8_t*)(boot_sector + 0x0D);
    boot->reserved_sectors = *(uint16_t*)(boot_sector + 0x0E);
    boot->fat_count = *(uint8_t*)(boot_sector + 0x10);
    boot->root_entries = *(uint16_t*)(boot_sector + 0x11);
    boot->total_sectors = *(uint16_t*)(boot_sector + 0x13);
    boot->sectors_per_fat = *(uint16_t*)(boot_sector + 0x16);
    
    if (boot->bytes_per_sector != 512) {
        return -3;
    }
    
    if (boot->sectors_per_cluster == 0 || boot->sectors_per_cluster > 128) {
        return -4;
    }
    
    if (boot->fat_count == 0 || boot->fat_count > 2) {
        return -5;
    }
    
    if (boot->root_entries == 0) {
        return -6;
    }
    
    if (boot->total_sectors == 0) {
        return -7;
    }
    
    if (boot->sectors_per_fat == 0) {
        return -8;
    }
    
    if (boot->total_sectors > 65535) {
        return -9;
    }
    
    if (boot->reserved_sectors == 0 || boot->reserved_sectors > boot->total_sectors / 4) {
        return -10;
    }
    
    if ((boot->sectors_per_cluster & (boot->sectors_per_cluster - 1)) != 0) {
        return -11;
    }
    
    uint32_t fat_sectors = (uint32_t)boot->fat_count * boot->sectors_per_fat;
    uint32_t root_sectors = (boot->root_entries * 32 + boot->bytes_per_sector - 1) / boot->bytes_per_sector;
    uint32_t min_sectors = boot->reserved_sectors + fat_sectors + root_sectors + 1;
    
    if (min_sectors > boot->total_sectors) {
        return -12;
    }
    
    if (boot->root_entries > 1024) {
        return -13;
    }
    
    return 0;
}

uint16_t get_fat12_entry(const uint8_t* fat_table, int fat_size_bytes, int cluster)
{
    if (cluster < 0) {
        return 0xFFFF;
    }
    
    int byte_offset = cluster * 3 / 2;
    
    if (byte_offset + 1 >= fat_size_bytes) {
        return 0xFFFF;
    }
    
    if (cluster % 2 == 0) {
        return (fat_table[byte_offset + 1] & 0x0F) << 8 | fat_table[byte_offset];
    } else {
        return fat_table[byte_offset + 1] << 4 | (fat_table[byte_offset] >> 4);
    }
}

uint8_t* load_fat_table(FILE *img, const BootSector *boot)
{
    uint8_t* fat_table = NULL;
    int fat_size_bytes = boot->sectors_per_fat * boot->bytes_per_sector;
    
    if (fat_size_bytes <= 0 || fat_size_bytes > 65536) {
        return NULL;
    }
    
    fat_table = malloc(fat_size_bytes);
    if (!fat_table) goto error;
    
    if (fseek(img, FAT_START(boot), SEEK_SET) != 0) goto error;
    if (fread(fat_table, fat_size_bytes, 1, img) != 1) goto error;
    
    return fat_table;
    
error:
    free(fat_table);
    return NULL;
}

void free_fat_table(uint8_t* fat_table)
{
    free(fat_table);
}

int set_fat12_entry(uint8_t* fat_table, int fat_size_bytes, int cluster, uint16_t value)
{
    if (cluster < 0) {
        return -1;
    }
    
    int byte_offset = cluster * 3 / 2;
    
    if (byte_offset + 1 >= fat_size_bytes) {
        return -1;
    }
    
    value &= 0xFFF;
    
    if (cluster % 2 == 0) {
        fat_table[byte_offset] = value & 0xFF;
        fat_table[byte_offset + 1] = (fat_table[byte_offset + 1] & 0xF0) | ((value >> 8) & 0x0F);
    } else {
        fat_table[byte_offset] = (fat_table[byte_offset] & 0x0F) | ((value & 0x0F) << 4);
        fat_table[byte_offset + 1] = (value >> 4) & 0xFF;
    }
    
    return 0;
}

int analyze_fat(FILE *img, const BootSector *boot, DiskInfo *info)
{
    int data_start_sectors = DATA_START(boot) / boot->bytes_per_sector;
    int total_data_sectors = boot->total_sectors - data_start_sectors;
    int total_clusters = total_data_sectors / boot->sectors_per_cluster;
    
    if (total_clusters > 4084) {
        return -1;
    }
    
    int fat_size_bytes = boot->sectors_per_fat * boot->bytes_per_sector;
    uint8_t fat_table[fat_size_bytes];
    
    if (fseek(img, FAT_START(boot), SEEK_SET) != 0) {
        return -3;
    }
    
    if (fread(fat_table, fat_size_bytes, 1, img) != 1) {
        return -4;
    }
    
    info->total_clusters = total_clusters;
    info->used_clusters = 0;
    info->free_clusters = 0;
    
    for (int cluster = 2; cluster < total_clusters + 2; cluster++) {
        uint16_t fat_entry = get_fat12_entry(fat_table, fat_size_bytes, cluster);
        
        if (fat_entry == 0x000) {
            info->free_clusters++;
        } else if (fat_entry == 0x001) {
            continue;
        } else if (fat_entry >= 0x002 && fat_entry <= 0xFF6) {
            info->used_clusters++;
        } else if (fat_entry == 0xFF7) {
            continue;
        } else if (fat_entry >= 0xFF8 && fat_entry <= 0xFFF) {
            info->used_clusters++;
        }
    }
    
    return 0;
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
