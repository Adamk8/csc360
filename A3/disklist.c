#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "Constants.h"


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

    fseek(fp, BLOCKSIZE_OFFSET, SEEK_SET);
    fread(&buffer, 2 , 1, fp);
    int block_size = htons(buffer);

    fseek(fp, ROOTDIRSTART_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    int root_start = htonl(buffer);

    fseek(fp, ROOTDIRBLOCKS_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    int root_blocks = htonl(buffer);

    int root_start_block = block_size * root_start;
    int max_dir_entries = (block_size/64) * root_blocks;
    uint8_t status;
    int file_size = -1;
    char *file_name = NULL;
    int year;
    uint8_t month,day,hour,minute,second;
    for (int i = 0; i < max_dir_entries; i++){
        fseek(fp, root_start_block + i*64, SEEK_SET);
        fread(&buffer, 1 , 1, fp);
        status = buffer; 
        if (status == 3){
            fseek(fp, root_start_block + i*64 + 9, SEEK_SET);
            fread(&buffer, 4, 1, fp);
            file_size = htonl(buffer);

            //Year
            fseek(fp, root_start_block + i*64 + 20, SEEK_SET);
            fread(&buffer, 2, 1, fp);
            year = htons(buffer);

            //Month 
            fseek(fp, root_start_block + i*64 + 22, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            month = buffer;

            //Day
            fseek(fp, root_start_block + i*64 + 23, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            day = buffer;

            //Hour
            fseek(fp, root_start_block + i*64 + 24, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            hour = buffer;

            //Minute
            fseek(fp, root_start_block + i*64 + 25, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            minute = buffer;

            //second 
            fseek(fp, root_start_block + i*64 + 26, SEEK_SET);
            fread(&buffer, 1, 1, fp);
            second = buffer;

            fseek(fp, root_start_block + i*64 + 27, SEEK_SET);
            fread(&char_buff, 31, 1, fp);
            file_name = char_buff;
            printf("F %10d %30s %04d/%02d/%02d %02d:%02d:%02d\n", file_size,file_name,year,month,day,hour,minute,second);
        }
        

    }


}