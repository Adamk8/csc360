#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "Constants.h"
#include "common.h"

FILE *fp; 

int main (int argc, char *argv[]) {
    char *filename = NULL;
    uint32_t buffer;
    char char_buff[256];
    if (argc >= 2){
        filename = argv[1];
    } else {
        printf("Please provide location for disk image\n"); 
        return -1;
    }
    fp = fopen(filename, "r");
    if (fp == NULL) printf("Failed to open file\n");

    int block_size = GetBlockSize(fp);
    int root_start = GetRootStart(fp);
    int root_blocks = GetRootBlocks(fp);

    int root_start_block = block_size * root_start;
    int max_dir_entries = (block_size/64) * root_blocks;
    uint8_t status;
    int file_size = -1;
    char *file_name = NULL;
    int year;
    uint8_t month,day,hour,minute,second;
    for (int i = 0; i < max_dir_entries; i++){
        fseek(fp, root_start_block + i*64, SEEK_SET);
        status = GetEntryStatus(fp);
        if (status == 3){
            fseek(fp, root_start_block + i*64 + DIRECTORY_FILE_SIZE_OFFSET, SEEK_SET);
            fread(&buffer, 4, 1, fp);
            file_size = htonl(buffer);

            //Year
            fseek(fp, root_start_block + i*64 + DIRECTORY_MODIFY_OFFSET, SEEK_SET);
            fread(&buffer, 2, 1, fp);
            year = htons(buffer);

            //Month 
            fseek(fp, root_start_block + i*64 + DIRECTORY_MODIFY_OFFSET + 2, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            month = buffer;

            //Day
            fseek(fp, root_start_block + i*64 + DIRECTORY_MODIFY_OFFSET + 3, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            day = buffer;

            //Hour
            fseek(fp, root_start_block + i*64 + DIRECTORY_MODIFY_OFFSET + 4, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            hour = buffer;

            //Minute
            fseek(fp, root_start_block + i*64 + DIRECTORY_MODIFY_OFFSET + 5, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            minute = buffer;

            //second 
            fseek(fp, root_start_block + i*64 + DIRECTORY_MODIFY_OFFSET + 6, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            second = buffer;

            fseek(fp, root_start_block + i*64 + DIRECTORY_FILENAME_OFFSET, SEEK_SET);
            fread(&char_buff, 31, 1, fp);
            file_name = char_buff;
            printf("F %10d %30s %04d/%02d/%02d %02d:%02d:%02d\n", file_size,file_name,year,month,day,hour,minute,second);
        }
        

    }

fclose(fp);
}