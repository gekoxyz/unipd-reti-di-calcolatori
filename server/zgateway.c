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
  char *name;
  char *value;
} headers[100];

struct sockaddr_in remote;

int main() {
  FILE *fin;
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

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int clientfd = accept(sockfd, (struct sockaddr *)&remote, &client_addr_size);
    if (clientfd == SOCKET_ERROR) {
      perror("Accept fallita");
      return 1;
    }

    if (fork() == 0) {
      commandline = headers[0].name = hbuf;
      // parsing the HTTP request to read all the headers and remove CRLFs
      for (j = 0, i = 0; read(clientfd, hbuf + i, 1); i++) {
        if ((hbuf[i] == ':') && (headers[j].value == NULL)) {
          headers[j].value = &hbuf[i + 1];
          hbuf[i] = 0;
        }
        if (hbuf[i] == '\n' && hbuf[i - 1] == '\r') {
          hbuf[i - 1] = 0;
          if (headers[j].name[0] == 0) break;
          headers[++j].name = &hbuf[i + 1];
        }
      }

      // printing the headers i just parsed
      for (i = 0; i < j; i++) {
        printf("%s --> %s\n", headers[i].name, headers[i].value);
      }

      char *method = commandline;
      for (i = 0; commandline[i] != ' '; i++) {
      }
      commandline[i] = 0;
      i++;

      // parsing the filename
      char *filename = commandline + i;
      for (; commandline[i] != ' '; i++) {
      }
      commandline[i] = 0;
      i++;
      // parsing the HTTP version (HTTP/1.1)
      char *ver = commandline + i;
      for (; commandline[i] != 0; i++) {
      }
      commandline[i] = 0;
      i++;
      printf("Method = %s, URI = %s, VER = %s \n", method, filename, ver);

      // check if filename starts with /exec/
      // if this is true save the output of the command to the exeout.txt file
      // and copy it to the string filename
      if (!strncmp("/exec/", filename, 6)) {
        sprintf(command, "%s > ./exeout.txt", filename + 6);
        printf("eseguo comando %s\n", command);
        system(command);
        strcpy(filename, "/exeout.txt");
      }

      // +1 is to remove the / at the beginning of /filename.html
      fin = fopen(filename + 1, "rt");
      // if i don't find the file i return 404 file not found
      if (fin == NULL) {
        sprintf(response, "HTTP/1.1 404 NOT FOUND\r\nConnection:close\r\n\r\n");
        // writing the header
        write(clientfd, response, strlen(response));
        fin = fopen("404.html", "r");
      } else {
        sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n<html>");
        // writing the header
        write(clientfd, response, strlen(response));
      }
      // writing the body
      while (!feof(fin)) {
        fread(entity, 1, 1000, fin);
        write(clientfd, entity, 1000);
      }
      fclose(fin);
      close(clientfd);
      exit(-1);
    } else {
      close(clientfd);
      continue;
    }
  }
}
