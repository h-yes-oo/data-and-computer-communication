#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define MAXBUF 1024

void* new_connection(void* sockfd){
  int client_sock = *(int *)sockfd;
  char buf[MAXBUF];
  char queue[MAXBUF];
  int str_len;
  printf("connect %d\n", client_sock);
  memset(queue, 0x00, MAXBUF);
  memset(buf, 0x00, MAXBUF);

  while(1) {
    str_len = read(client_sock, buf, MAXBUF);
    if(!strcmp(buf, "RECV\n")){
      strcat(queue,"LAST_MSG\n");
      write(client_sock, queue, strlen(queue));
      memset(queue, 0x00, MAXBUF);
      break;
    } else {
      strcat(queue, buf);
      memset(buf, 0x00, MAXBUF);
    }
  }
  close(client_sock);
  printf("disconnect %d\n",client_sock);
  pthread_exit((void*)0);
}

int main(int argc, char **argv){
  int server_sockfd, sockfd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len;

  if( 0 > (server_sockfd = socket(AF_INET, SOCK_STREAM, 0))) {
    printf("Error creating socket\n");
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(3050);

  if (0 > (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))) {
    printf("Error binding\n");
  }
  
  if(0 > listen(server_sockfd, 5)){
    printf("Error listening\n");
  }

  pthread_t conn_thread;
  while(1){
    client_addr_len = sizeof(client_addr);
    if (0 > (sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &client_addr_len))){
      printf("Error accepting\n");
    } else {
      pthread_create(&conn_thread, NULL, new_connection, (void *) &sockfd);
      pthread_detach(conn_thread);
    }
  }
}
