#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

struct sockaddr_in srvaddr, remote;

int main() {
  char request[3001] = {0};
  char response[3001] = {0};

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("Socket fallita");
    return -1;
  }
  srvaddr.sin_family = AF_INET;
  srvaddr.sin_port = htons(8080);
  srvaddr.sin_addr.s_addr = 0;

  int t = bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr_in));
  if (t == -1) {
    perror("Bind fallita");
    return -1;
  }
  t = listen(sockfd, 5);
  if (t == -1) {
    perror("Listen fallita");
    return -1;
  }
  while (1) {
    int len = sizeof(struct sockaddr);
    int s2 = accept(sockfd, (struct sockaddr *)&remote, &len);

    if (fork()) continue;
    if (s2 == -1) {
      perror("Accept fallita");
      return -1;
    }

    t = read(s2, request, 3000);
    if (t == -1) {
      perror("Read fallita");
      return -1;
    }

    request[t] = 0;
    printf("Richiesta:\n%s", request);
    sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n<html><h1>FUNZIONO!</h1></html>");
    write(s2, response, strlen(response));
    close(s2);
    exit(1);
  }

  return 0;
}