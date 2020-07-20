#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "Constants.h"


FILE *fp; 
FILE *source, *output_file;

int main (int argc, char *argv[]) {
    char *filename = NULL;
    char *target_file = NULL;
    uint32_t buffer;
    char char_buff[512];
    if (argc >= 3){
        filename = argv[1];
        target_file = argv[2];
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

    fseek(fp, FATSTART_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    int fat_start = htonl(buffer);

    int root_start_block = block_size * root_start;
    int max_dir_entries = (block_size/64) * root_blocks;
    uint8_t status;
    int found = 0;
    char *file_name = NULL;
    for (int i = 0; i < max_dir_entries; i++){
        fseek(fp, root_start_block + i*64, SEEK_SET);
        fread(&buffer, 1 , 1, fp);
        status = buffer; 
        if (status == 3){
            fseek(fp, root_start_block + i*64 + 27, SEEK_SET);
            fread(&char_buff, 31, 1, fp);
            file_name = char_buff;
            if (strcmp(file_name, target_file) == 0){
                found = 1;
                output_file = fopen(target_file, "w");

                fseek(fp, root_start_block + i*64 + 1, SEEK_SET);
                fread(&buffer, 4, 1, fp);
                int next_file_block = htonl(buffer);

                fseek(fp, root_start_block + i*64 + 9, SEEK_SET);
                fread(&buffer, 4, 1, fp);
                int file_size = htonl(buffer);

                while(next_file_block != FAT_EOF){
                    fseek(fp, next_file_block*block_size, SEEK_SET);
                    if (file_size < 512){
                        fread(&char_buff, file_size, 1, fp);
                        fwrite(char_buff, 1, file_size, output_file);
                    }
                    else {
                        fread(&char_buff, block_size, 1, fp);
                        fwrite(char_buff, 1, block_size, output_file);
                        file_size = file_size - 512;
                    }

                    fseek(fp, fat_start*block_size + 4*next_file_block, SEEK_SET);
                    fread(&buffer, 4, 1, fp);
                    next_file_block = htonl(buffer);
                }

            }
        }
    }
    if(!found){
        printf("File Not Found\n");
    }
    fclose(output_file);
    fclose(fp);
}