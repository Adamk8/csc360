#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "Constants.h"
#include "common.h"

uint32_t buffer;
uint8_t buffer_short;
char char_buff[512];

int GetBlockSize(FILE *fp){
    fseek(fp, BLOCKSIZE_OFFSET, SEEK_SET);
    fread(&buffer, 2 , 1, fp);
    return htons(buffer);
}

int GetBlockCount(FILE *fp){
    fseek(fp, BLOCKCOUNT_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    return htonl(buffer);
}

int GetRootStart(FILE *fp){
    fseek(fp, ROOTDIRSTART_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    return htonl(buffer);
}

int GetRootBlocks(FILE *fp){
    fseek(fp, ROOTDIRBLOCKS_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    return htonl(buffer);
}

int GetFatStart(FILE *fp){
    fseek(fp, FATSTART_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    return htonl(buffer);
}

int GetFatBlocks(FILE *fp){
    fseek(fp, FATBLOCKS_OFFSET, SEEK_SET);
    fread(&buffer, 4, 1, fp);
    return htonl(buffer);
}

int GetFatStatus(FILE *fp){
    fread(&buffer, 4, 1, fp);
    return htonl(buffer);
}

uint8_t GetEntryStatus(FILE *fp){
    fread(&buffer_short, 1 , 1, fp);
    return buffer_short;
}

char *GetFileName(FILE *fp){
    fread(&char_buff, 31, 1, fp);
    return char_buff;
}
