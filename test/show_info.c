#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "block_pc.h"
#include "block.h"
#include "partition.h"
#include "gristle.h"

extern struct fat_info fatfs;

int main(int argc, char *argv[]) {
    int mounted = 0;
    uint8_t *fsbuf = NULL;
    int i;
    int r;
    struct partition *part_list;
    blockno_t image_size = 0;
    
    if(argc < 2) {
        printf("Please specify the image file to use.\n");
        exit(-2);
    }
    
    if(argc > 2) {
        image_size = strtoul(argv[2], NULL, 10);
    }

    block_pc_set_image_name(argv[1]);
    
    if(block_init() == 0) {
        // attempt to mount the card root
        if(fat_mount(0, (image_size ? image_size : block_get_volume_size()), 0)) {
            // root mount failed, try and read a partition table
            fsbuf = (uint8_t *)malloc(512);
            block_read(0, fsbuf);
            r = read_partition_table(fsbuf, (image_size ? image_size : block_get_volume_size()), &part_list);
            if(r > 0) {
                for(i=0;i<r;i++) {
                    if(fat_mount(part_list[i].start, part_list[i].length, part_list[i].type) == 0) {
                    mounted = 1;
                    break;
                    }
                }
            }
        } else {
            printf("Mounted root partition.\n");
            mounted = 1;
        }
    }
    if(fsbuf != NULL) {
        free(fsbuf);
    }
    if(mounted) {
        printf("Image mounted successfully.\n");
        
        printf("read_only: %d\n", fatfs.read_only);
        printf("fat_entry_len: %d\n", fatfs.fat_entry_len);
        printf("end_cluster_marker: 0x%x\n", fatfs.end_cluster_marker);
        printf("sectors_per_cluster: %d\n", fatfs.sectors_per_cluster);
        printf("cluster0: %d\n", fatfs.cluster0);
        printf("active_fat_start: %d blocks (0x%x bytes)\n", fatfs.active_fat_start, fatfs.active_fat_start * block_get_block_size());
        printf("sectors_per_fat: %d\n", fatfs.sectors_per_fat);
        printf("root_len: %d\n", fatfs.root_len);
        printf("root_cluster: %d\n", fatfs.root_cluster);
        if(fatfs.type == 0x0b) {
            printf("type: 0b (FAT32)\n");
        } else if(fatfs.type == 0x06) {
            printf("type: 06 (FAT16)\n");
        } else {
            printf("type: %02x (\?\?)\n", fatfs.type);
        }
        printf("part_start: %d blocks (0x%x bytes)\n", fatfs.part_start, fatfs.part_start * block_get_block_size());
        printf("total_sectors: %d\n", fatfs.total_sectors);
    }
    block_halt();
  
    exit(0);
}

