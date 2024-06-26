#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define SOCKET_ERROR -1

char command[1000];
char hbuf[10000];
char entity[1000];
struct headers {
  char *n;
  char *v;
} h[100];

struct sockaddr_in remote;

int main() {
  FILE *fin;
  char *method, *filename, *ver;
  char request[3001];
  char response[3001];
  int t, len;
  int yes = 1;
  char *commandline;
  int i, j;

  // creating sockfd, with:
  // socket family AF_INET ipv4 sockets
  // socket type SOCK_STREAM connection oriented sockets
  // 0 is the default protocol for AF_INET and SOCK_STREAM (TCP)
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == SOCKET_ERROR) {
    perror("Socket fallita");
    return 1;
  }

  // allow the server to restart and bind to the same address and port
  // without waiting for the previous connection to time out.
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR) {
    perror("setsockopt fallita");
    return 1;
  }

  struct sockaddr_in server_addr = {
    // IPv4 address
    .sin_family = AF_INET,
    // set port by using htons to convert from host byte order to network byte order
    .sin_port = htons(SERVER_PORT)};

  // bind is used to associate a socket with a specific address and port
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
    perror("Bind fallita");
    return 1;
  }

  // listen marks  the  socket  referred  to by sockfd as a passive socket, that is, as a
  // socket that will be used to accept incoming connection requests using accept(2)
  if (listen(sockfd, 5) == SOCKET_ERROR) {
    perror("Listen fallita");
    return 1;
  }

  printf("Server running on port %d\n", SERVER_PORT);
  int clientfd;
  while (1) {
    close(clientfd);
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    clientfd = accept(sockfd, (struct sockaddr *)&remote, &client_addr_size);

    if (fork()) continue;
    if (clientfd == SOCKET_ERROR) {
      perror("Accept fallita");
      return 1;
    }
    commandline = h[0].n = hbuf;
    for (j = 0, i = 0; read(clientfd, hbuf + i, 1); i++) {
      if ((hbuf[i] == ':') && (h[j].v == NULL)) {
        h[j].v = &hbuf[i + 1];
        hbuf[i] = 0;
      }
      if (hbuf[i] == '\n' && hbuf[i - 1] == '\r') {
        hbuf[i - 1] = 0;
        if (h[j].n[0] == 0) break;
        h[++j].n = &hbuf[i + 1];
      }
    }

    for (i = 0; i < j; i++) {
      printf("%s ----> %s\n", h[i].n, h[i].v);
    }
    method = commandline;
    for (i = 0; commandline[i] != ' '; i++) {
    }
    commandline[i] = 0;
    i = i + 1;
    filename = commandline + i;
    for (; commandline[i] != ' '; i++) {
    }
    commandline[i] = 0;
    i = i + 1;
    ver = commandline + i;
    for (; commandline[i] != 0; i++) {
    }
    commandline[i] = 0;
    i = i + 1;
    printf("Method = %s, URI = %s, VER = %s \n", method, filename, ver);
    if (!strncmp("/exec/", filename, 6)) {
      sprintf(command, "%s > ./exeout.txt", filename + 6);
      printf("eseguo comando %s\n", command);
      system(command);
      strcpy(filename, "/exeout.txt");
    }
    fin = fopen(filename + 1, "rt");
    if (fin == NULL) {
      sprintf(response, "HTTP/1.1 404 NOT FOUND\r\nConnection:close\r\n\r\n<html><h1>File %s non trovato</h1>i</html>", filename);
      write(clientfd, response, strlen(response));
      close(clientfd);
      exit(1);
    }
    sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n<html>");
    write(clientfd, response, strlen(response));
    while (!feof(fin)) {
      fread(entity, 1, 1000, fin);
      write(clientfd, entity, 1000);
    }
    fclose(fin);
    close(clientfd);
    exit(-1);
  }
}
