#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8080
#define SOCKET_ERROR -1

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

char hbuf[10000];
char entity[1000];

struct header {
  char *name;
  char *value;
} headers[100];

struct sockaddr_in remote;

int main() {
  FILE *fin;
  char request[3001];
  char response[3001];
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

  struct sockaddr_in server_addr = {
    // the address is an IPv4 address
    .sin_family = AF_INET,
    // sets the port of the structure by using htons to converto from host byte order to network byte order
    .sin_port = htons(SERVER_PORT)};

  // SOL_SOCKET is the socket layer itself. It is used for options that are protocol independent
  // SO_REUSEADDR controls if bind should permit reuse of local addresses for this socket
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR) {
    perror("setsockopt fallita");
    return 1;
  }

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

  /*
  Inviare entity body usando transfer coding chuncked, come descritto dalla grammatica riportata
  nella sezione 3.6.1 della RFC 2616, evitando tutte le parti opzionali.
  */

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (clientfd == SOCKET_ERROR) {
      perror("Accept fallita");
      return 1;
    }

    if (fork() == 0) {
      char *commandline = headers[0].name = hbuf;
      // parsing the HTTP request to read all the headers and remove CRLFs
      for (j = 0, i = 0; read(clientfd, hbuf + i, 1); i++) {
        if ((hbuf[i] == ':') && (headers[j].value == NULL)) {
          hbuf[i] = 0;
          headers[j].value = &hbuf[i + 1];
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
      for (i = 0; commandline[i] != ' '; i++)
        ;
      commandline[i] = 0;
      i++;
      // parsing the filename (index.html typically)
      char *filename = commandline + i;
      for (; commandline[i] != ' '; i++)
        ;
      commandline[i] = 0;
      i++;
      // parsing the HTTP version (HTTP/1.1)
      char *ver = commandline + i;
      for (; commandline[i] != 0; i++)
        ;
      commandline[i] = 0;
      i++;
      printf("Method = %s, URI = %s, VER = %s \n", method, filename, ver);

      // +1 is to remove the / at the beginning of /filename.html
      fin = fopen(filename + 1, "r");
      // if i don't find the file i return 404 file not found
      if (fin == NULL) {
        sprintf(response, "HTTP/1.1 404 NOT FOUND\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n");
        // writing the header
        write(clientfd, response, strlen(response));
        fin = fopen("404.html", "r");
      } else {
        sprintf(response, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nConnection: keep-alive\r\n\r\n");
        // writing the header
        write(clientfd, response, strlen(response));
      }

      // move to the end of the file to determine its size and rewind the file pointer back
      // to the beginning of the file
      fseek(fin, 0L, SEEK_END);
      int file_size = ftell(fin);
      rewind(fin);

      int t = -1;
      char file_buffer[2048] = {0};
      // fill the buffer, send to the client the amount of data in the buffer and 
      for (int k = 0; (t = fread(file_buffer, sizeof(char), 2048, fin)) > 0; k += t) {
        // chunk size
        sprintf(response, "%x\r\n", t);
        write(clientfd, response, strlen(response));
        // chunk
        write(clientfd, file_buffer, t);
        // CRLF
        sprintf(response, "\r\n");
        write(clientfd, response, strlen(response));
      }
      // send the empty chunk as specified in the RFC
      sprintf(response, "0\r\n\r\n");
      write(clientfd, response, strlen(response));

      fclose(fin);
      close(clientfd);
      exit(-1);
    } else {
      close(clientfd);
      continue;
    }
  }
  close(sockfd);
}
