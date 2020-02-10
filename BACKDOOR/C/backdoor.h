#ifndef MY_SOCK_H
#define MY_SOCK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define PORT 9000    
#define STATE_LEN 3
void addr_init(struct sockaddr_in *addr, int port, long ip){
addr->sin_family= AF_INET;
addr->sin_port = htons(port);
addr->sin_addr.s_addr = ip;
}

int fileLen(FILE *fp){
char buff;
int len=0;
rewind(fp);
while(!feof(fp)) {
buff = fgetc(fp);
len++;
}
rewind(fp);
return len;
}

char *readFile(char *file){
FILE *fp;
int i;
char *buff;
fp = fopen(file,"r");
buff = (char*) malloc(fileLen(fp)*sizeof(char));
for (i=0; !feof(fp); i++)
buff[i] = fgetc(fp);
buff[i-1] = '\0';
fclose(fp);
return buff;
}
#endif
