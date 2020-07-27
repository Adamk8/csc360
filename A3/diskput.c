#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Constants.h"
#include "common.h"
#include <time.h>


FILE *fp; 
FILE *input_file;
uint32_t buffer;
int final_block_count = 0;

//Reserve fat blocks and return the start block
int ReserveFatBlocks(int fat_start, int file_size, int block_size){
    int fat_status = -1;
    int i = -1; 
    int fat_entry = -1;
    int previous_block = -1;
    int start_block = -1;
    int fat_start_block = fat_start*block_size;
    int fat_write;
    while (file_size > 0){
        final_block_count++;
        while(fat_status != FAT_FREE){
            i++;
            fseek(fp, fat_start_block + i*4, SEEK_SET);
            fread(&buffer, 4, 1, fp);
            fat_status = htonl(buffer);
        }
        fat_entry = i;
        //printf("Next Block: %d\n", fat_entry);
        if(previous_block == -1){
            start_block = fat_entry;
        }
        if (previous_block != -1){
            fseek(fp, fat_start*block_size + 4*previous_block, SEEK_SET);
            fat_write = ntohl(fat_entry);
            fwrite(&fat_write, 1,4,fp);
        }
        previous_block = fat_entry;
        file_size = file_size - block_size;
        i++;
    }
    fseek(fp, fat_start*block_size + 4*fat_entry, SEEK_SET);
    int end = FAT_EOF;
    end = ntohl(end);
    fwrite(&end,1,4,fp);
    return start_block;
}

void WriteFileContents(int start_block, int fat_start, int file_size, int block_size){
    int file_block = start_block;
    char char_buff[block_size];
    int i = 0;
    while (file_block != FAT_EOF){
        //Get input
        fseek(input_file, i*block_size, SEEK_SET);
        if (file_size < block_size){
            fread(&char_buff, file_size, 1, input_file);
        } else {
            fread(&char_buff, block_size, 1, input_file);
        }      

        //Get output position
        fseek(fp, file_block*block_size, SEEK_SET);
        //Write
        if (file_size < block_size){
            fwrite(char_buff, 1, file_size, fp);
        } else {
            fwrite(char_buff, 1, block_size, fp);
        }
        fseek(fp, file_block*block_size, SEEK_SET);
        fread(&char_buff, block_size, 1, fp);

        fseek(fp, fat_start*block_size + 4*file_block, SEEK_SET);
        fread(&buffer,4,1,fp);
        file_block = htonl(buffer);
        file_size = file_size - block_size;
        i++;
    }
}


void SetDirectoryEntry(int entry_address, 
    int start_block, 
    int block_count, 
    int file_size,
    char *filename,
    struct tm time)
    {
    uint8_t file_status = 3;

    //Status 
    fseek(fp, entry_address, SEEK_SET);
    fwrite(&file_status,1,1,fp);
    
    //Start
    fseek(fp, entry_address + DIRECTORY_START_BLOCK_OFFSET, SEEK_SET);
    start_block = ntohl(start_block);
    fwrite(&start_block,1,4,fp);
    fseek(fp, entry_address + DIRECTORY_START_BLOCK_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);

    //Block Count
    fseek(fp, entry_address + DIRECTORY_BLOCK_COUNT_OFFSET, SEEK_SET);
    block_count = ntohl(block_count);
    fwrite(&block_count,1,4,fp);

    //File Size 
    fseek(fp, entry_address + DIRECTORY_FILE_SIZE_OFFSET, SEEK_SET);
    file_size = ntohl(file_size);
    fwrite(&file_size,1,4,fp);

    //Year
    fseek(fp, entry_address + DIRECTORY_MODIFY_OFFSET, SEEK_SET);
    int year = ntohl(time.tm_year);
    fwrite(&year,1,2,fp);

    //Month 
    fseek(fp, entry_address + DIRECTORY_MODIFY_OFFSET + 2, SEEK_SET);
    fwrite(&(time.tm_mon),1,1,fp);

    //Day
    fseek(fp, entry_address + DIRECTORY_MODIFY_OFFSET + 3, SEEK_SET);
    fwrite(&(time.tm_mday),1,1,fp);

    //Hour
    fseek(fp, entry_address + DIRECTORY_MODIFY_OFFSET + 4, SEEK_SET);
    fwrite(&(time.tm_hour),1,1,fp);

    //Minute
    fseek(fp, entry_address + DIRECTORY_MODIFY_OFFSET + 5, SEEK_SET);
    fwrite(&(time.tm_min),1,1,fp);

    //second 
    fseek(fp, entry_address + DIRECTORY_MODIFY_OFFSET + 6, SEEK_SET);
    fwrite(&(time.tm_sec),1,1,fp);

    //filename
    fseek(fp, entry_address + DIRECTORY_FILENAME_OFFSET, SEEK_SET);
    fwrite(filename, 1, 31, fp);
}

int main (int argc, char *argv[]) {
    char *disk_image = NULL;
    char *input= NULL;
    if (argc >= 3){
        disk_image = argv[1];
        input = argv[2];
    } else {
        printf("Please provide location for disk image\n"); 
        return -1;
    }
    fp = fopen(disk_image, "r+");
    input_file = fopen(input, "r");
    if (fp == NULL || input_file == NULL){
        printf("Failed to open file\n");
        return -1;
    } 

    //get file info 
    struct stat file_info;
    stat(input, &file_info);


    int block_size = GetBlockSize(fp);
    int root_start = GetRootStart(fp);
    int root_blocks = GetRootBlocks(fp);
    int fat_start = GetFatStart(fp);

    char char_buff[block_size];

    int root_start_block = block_size * root_start;
    int max_dir_entries = (block_size/64) * root_blocks;
    uint8_t status;
    char *file_name = NULL;

    //check if file already exists
    for (int i = 0; i < max_dir_entries; i++){
        fseek(fp, root_start_block + i*64, SEEK_SET);
        status = GetEntryStatus(fp);
        if (status == 3){
            fseek(fp, root_start_block + i*64 + DIRECTORY_FILENAME_OFFSET, SEEK_SET);
            fread(&char_buff, 31, 1, fp);
            file_name = char_buff;
            if(strcmp(file_name, input) == 0){
                printf("Error: File already exists\n");
                return -1;
            }
        }
    }


    int file_start_block = ReserveFatBlocks(fat_start, file_info.st_size, block_size);
    WriteFileContents(file_start_block,fat_start, file_info.st_size, block_size);
    time_t t = time(NULL);
    struct tm time = *localtime(&t);

    //Create directory entry
    for (int i = 0; i < max_dir_entries; i++){
        fseek(fp, root_start_block + i*64, SEEK_SET);
        status = GetEntryStatus(fp); 
        if (status == 0){
            SetDirectoryEntry(root_start_block + i*64,  
                file_start_block,
                final_block_count,
                file_info.st_size,
                input,
                time);
            break;
        }
    }
    fclose(fp);
    fclose(input_file);
}