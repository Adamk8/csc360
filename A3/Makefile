CFLAGS = -Wall -g   

all: diskinfo disklist diskget diskput 

diskinfo: diskinfo.c common.c
	gcc $(CFLAGS) diskinfo.c common.c -o diskinfo

disklist: disklist.c common.c
	gcc $(CFLAGS) disklist.c common.c -o disklist

diskget: diskget.c common.c
	gcc $(CFLAGS) diskget.c common.c -o diskget
	
diskput: diskput.c common.c
	gcc $(CFLAGS) diskput.c common.c -o diskput

