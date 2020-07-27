#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "Constants.h"
#include "common.h"

FILE *fp; 

int main (int argc, char *argv[]) {
    char *filename = NULL;
    if (argc >= 2){
        filename = argv[1];
    } else {
        printf("Please provide location for disk image\n"); 
        return -1;
    }
    fp = fopen(filename, "r");
    if (fp == NULL){
        printf("Failed to open file\n");
        return -1;
    } 
    
    int block_size = GetBlockSize(fp);
    int block_count = GetBlockCount(fp);
    int fat_start = GetFatStart(fp);
    int fat_blocks = GetFatBlocks(fp);
    int root_start = GetRootStart(fp);
    int root_blocks = GetRootBlocks(fp);

    printf("Super block information:\n");
    printf("Block size: %d\n", block_size);
    printf("Block count: %d\n", block_count);
    printf("FAT starts: %d\n", fat_start);
    printf("FAT blocks: %d\n", fat_blocks);
    printf("Root directory start: %d\n", root_start);
    printf("Root directory blocks: %d\n", root_blocks);

    int free_blocks = 0;
    int reserved_blocks = 0;
    int allocated_blocks = 0; 
    int status = -1; 
    int fat_start_block = block_size*fat_start;
    fseek(fp, fat_start_block, SEEK_SET);
    for (int i = 0; i < block_count; i++){
        status = GetFatStatus(fp);
        if (status == FAT_FREE) free_blocks++;
        else if (status == FAT_RESERVED) reserved_blocks++;
        else allocated_blocks++;
    }
    printf("\nFAT information:\n");
    printf("Free Blocks: %d\n", free_blocks);
    printf("Reserved Blocks: %d\n", reserved_blocks);
    printf("Allocated Blocks: %d\n", allocated_blocks);
    fclose(fp);
}