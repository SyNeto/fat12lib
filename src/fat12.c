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

int analyze_fat(FILE *img, const BootSector *boot, DiskInfo *info)
{
    fseek(img, FAT_START(boot), SEEK_SET);
    int total_clusters = (boot->total_sectors - DATA_START(boot) / boot->bytes_per_sector) / boot-> sectors_per_cluster;
    info->total_clusters = total_clusters;
    info->used_clusters = 0;
    info->free_clusters = 0;

    uint16_t entry;
    for (int i = 0; i < total_clusters; i++) {
        fread(&entry, sizeof(uint16_t), 1, img);
        if(entry == 0x000) {
            info->free_clusters++;
        } else if (entry < 0xFF8) {
            info->used_clusters++;
        }
    }
    return 0;
}

void print_disk_info(const DiskInfo *info)
{
    printf("Tamaño del sector: %d bytes\n", info->boot_sector.bytes_per_sector);
    printf("Sectores por clúster: %d\n", info->boot_sector.sectors_per_cluster);
    printf("Número de FATs: %d\n", info->boot_sector.fat_count);
    printf("Entradas del directorio raíz: %d\n", info->boot_sector.root_entries);
    printf("Total de clústeres: %d\n", info->total_clusters);
    printf("Clústeres usados: %d\n", info->used_clusters);
    printf("Clústeres libres: %d\n", info->free_clusters);
}
