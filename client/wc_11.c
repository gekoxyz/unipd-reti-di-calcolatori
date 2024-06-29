#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netdb.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define SOCKET_ERROR -1
#define SERVER_PORT 80

char hbuf[10000] = {0};

struct body {
  int chunk_size;
  char *content;
} body_elements[20];

struct header {
  char *name;
  char *value;
} headers[100];

int main() {
  int t, i, j;
  FILE *f;
  struct sockaddr_in server;
  unsigned char *p;

  // creating my_socket, with:
  // socket family AF_INET ipv4 sockets
  // socket type SOCK_STREAM connection oriented sockets
  // 0 is the default protocol for AF_INET and SOCK_STREAM (TCP)
  int my_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (my_socket == SOCKET_ERROR) {
    printf("Errno = %d (%d)\n", errno, EAFNOSUPPORT);
    perror("Socket fallita:");
    return 1;
  }
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);
  struct hostent *he; he = gethostbyname("gekohomelab.ddns.net");
  server.sin_addr.s_addr = *(unsigned int *)(he->h_addr);

  if (connect(my_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
    perror("Connect fallita");
    return 1;
  }

  // writing the HTTP request
  char *request = "GET /index.html HTTP/1.1\r\n\r\n";
  // writing the request to the socket
  write(my_socket, request, strlen(request));

  // assign the address of hbuf
  headers[0].name = hbuf;

  // we do a read on the socket of the element of the buffer
  for (j = 0, i = 0; read(my_socket, hbuf + i, 1); i++) {
    // if we find : we have to put the pointer to the next character and
    // substitute the : with 0 the first : are those which determine the
    // instantiation of the value
    if ((hbuf[i] == ':') && (headers[j].value == NULL)) {
      hbuf[i] = 0;
      headers[j].value = &hbuf[i + 1];
    }
    // if the CRLF is found, the pointer is sete to the next character and
    // substitute the CRLF with 0
    if (hbuf[i] == '\n' && hbuf[i - 1] == '\r') {
      hbuf[i - 1] = 0;
      // check that the previous character wasn't null
      if (headers[j].name[0] == 0) break;
      headers[++j].name = &hbuf[i + 1];
    }
  }

  int len = 0;
  for (i = 0; i < j; i++) {
    printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET " --> " ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "\n", headers[i].name, headers[i].value);
    if (!strcmp(headers[i].name, "Content-Length")) {
      // atoi converts a string of characters into an integer value
      len = atoi(headers[i].value);
      printf("len = %d\n", len);
    }
  }

  char bbuf[2000000] = {0};
  // if Content-Length is not there i have a chunk to read
  if (len == 0) {
    int i = 0;
    char *chunk_start = &bbuf[0];
    // read 1 char at a time
    while (read(my_socket, &bbuf[i], 1)) {
      // check if CRLF has been found
      if (bbuf[i] == '\n' && bbuf[i - 1] == '\r') {
        // remove the \r
        bbuf[i - 1] = 0;
        // move from \n to the next position in the buffer (which is free)
        i++;
        // convert from pointer of chunk_start to the next char which
        // is not hexadecimal (this means the \r) the number and get the
        // chunk length
        long size = strtol(chunk_start, NULL, 16);
        printf("conversion: bbuf[%d] -> %ld\n", i, size);
        if (size == 0) break;
        // add 2 which is the \r\n so i will end up exactly at the start of the next chunk-length
        size += 2;
        // read all the body from the buffer

        for (long k = 0; k < size; k++) {
          read(my_socket, &bbuf[i + k], 1);
        }
        // increment the counter of i since i read all this characters
        i += size;
        // if i am after \r\n but not in a chunk length this mean
        // that the next char is the FIRST char of the chunk length
        // so i take that point as my first point which will then be converted
        chunk_start = &bbuf[i] + 1;
      }
      i++;
    }
  }

  printf("\nPrinting the parsed data\n\n");
  for (int i = 0; i < 1999999; i++) {
    printf("%c", bbuf[i]);
  }
  printf("\n\n\n");

  return 0;
}
