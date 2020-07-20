#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "Constants.h"


FILE *fp; 

int main (int argc, char *argv[]) {
    char *filename = NULL;
    uint32_t buffer;
    if (argc >= 2){
        filename = argv[1];
    } else {
        printf("Please provide location for disk image\n"); 
        return -1;
    }
    fp = fopen(filename, "r");
    if (fp == NULL) printf("Failed to open file\n");
    fseek(fp, BLOCKSIZE_OFFSET, SEEK_SET);
    fread(&buffer, 2 , 1, fp);
    int block_size = htons(buffer);

    fseek(fp, BLOCKCOUNT_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    int block_count = htonl(buffer);

    fseek(fp, FATSTART_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    int fat_start = htonl(buffer);

    fseek(fp, FATBLOCKS_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    int fat_blocks = htonl(buffer);

    fseek(fp, ROOTDIRSTART_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    int root_start = htonl(buffer);

    fseek(fp, ROOTDIRBLOCKS_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    int root_blocks = htonl(buffer);

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
        fread(&buffer, 4, 1, fp);
        status = htonl(buffer);
        if (status == FAT_FREE) free_blocks++;
        else if (status == FAT_RESERVED) reserved_blocks++;
        else allocated_blocks++;
    }
    printf("\nFAT information:\n");
    printf("Free Blocks: %d\n", free_blocks);
    printf("Reserved Blocks: %d\n", reserved_blocks);
    printf("Allocated Blocks: %d\n", allocated_blocks);
}