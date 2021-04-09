#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAXBUF 1024

int main(int argc, char **argv){
  struct sockaddr_in server_addr;
  int sockfd;
  socklen_t server_addr_len;
  char buf[MAXBUF];
  int str_len;

  if ( 0 > (sockfd=socket(AF_INET, SOCK_STREAM, 0)))
  {
    printf("Error creating socket\n");
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(3050);
  server_addr_len = sizeof(server_addr);

  connect(sockfd, (struct sockaddr *) &server_addr, server_addr_len);
  memset(buf, 0x00, MAXBUF);

  while(1){
    fgets(buf, MAXBUF, stdin);

    if(!strcmp(buf, "Q\n"))
      break;
    write(sockfd, buf, strlen(buf));

    str_len = read(sockfd, buf, MAXBUF-1);
    buf[str_len] = 0;
    printf("%s",buf);
  }
  close(sockfd);
  return 0;
}
