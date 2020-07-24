/* Header file for common.c */

int GetBlockSize(FILE *fp);

int GetBlockCount(FILE *fp);

int GetRootStart(FILE *fp);

int GetRootBlocks(FILE *fp);

int GetFatStart(FILE *fp);

int GetFatBlocks(FILE *fp);

int GetFatStatus(FILE *fp);

uint8_t GetEntryStatus(FILE *fp);

char *GetFileName(FILE *fp);



