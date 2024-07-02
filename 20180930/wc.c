/*
gestire il meccanismo di controllo del caching basato sugli header Last-Modified
e If-Modified-Since descritti nella RFC2616 ai capitoli 14.29, 14.25.

Il programma wc18.c modificato, per la verifica del corretto funzionamento, dovrà essere azionato due volte.
La prima volta, a cache vuota, dovrà acquisire la data di ultima modifica della risorsa e la salverà insieme al entity
body sulla cache (file su disco).
La seconda volta (e le successive) ri-scaricherà la risorsa solo se è questa è stata modificata nel frattempo sul
server, altrimenti dovrà accedere alla copia in cache.

Pseudocodice:
1.a) If resource is already present in dir ./cache/
    GET www.example.com If-Modified-Since:httpDate(timestamp)
    --> Response
    HTTP/1.1 304 Not Modified --> return cached
1.b) Else resource not present
    GET www.example.com
--> Response
Else) HTTP/1.1 200 OK --> Download the resource and save it in dir ./cache/
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

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
  // struct hostent *he; he = gethostbyname("gekohomelab.ddns.net");
  struct hostent *he;
  he = gethostbyname("example.com");
  server.sin_addr.s_addr = *(unsigned int *)(he->h_addr);

  if (connect(my_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
    perror("Connect fallita");
    return 1;
  }

  char cached_file_name[1024] = {0};
  char *filename = "index.html";
  strcpy(cached_file_name, "./cache/");
  strcat(cached_file_name, filename);

  FILE *cached_file;

  // if ((cached_file = fopen(cached_file_name, "r")) != NULL) {
  //   printf(ANSI_COLOR_YELLOW "Resource already downloaded, verifying cache\n" ANSI_COLOR_RESET);
  //   // the first line of the cached file contains the timestamp of the last modification date of the resource
  //   char cache_timestamp[16] = {0};
  //   fread(cache_timestamp, sizeof(char), 16, cached_file);

  //   printf("CACHE TIMESTAMP = %s\n", cache_timestamp);

  //   // HTTP DATE
  //   // Thu, 17 Oct 2019 07:18:26 GMT
  //   // %a, %d %b %Y %T=%H:%M:%S GMT=%Z
  //   char *format = "%a, %d %b %Y %T %Z";  // man 3 strptime

  //   time_t epoch_saved = atoi(cache_timestamp);
  //   struct tm tmSaved = *gmtime(&epoch_saved);
  //   char httpDateCache[16];
  //   strftime(httpDateCache, sizeof(httpDateCache), format, &tmSaved);
  //   printf("Last time of download is %s\n", httpDateCache);

  //   exit(123);
  // } else {
    printf(ANSI_COLOR_RED "Resource not in cache, downloading\n" ANSI_COLOR_RESET);
    
    // writing the HTTP request
    char request[1024] = {0};
    sprintf(request, "GET /%s HTTP/1.1\r\n\r\n", filename);
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
        headers[++j].name = &hbuf[i + 2];
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


    
    cached_file = fopen(cached_file_name, "w+");

    // save the timestamp in the cached file
    time_t unix_timestamp = time(NULL);
    fprintf(cached_file, "%lu\n", (unsigned long)unix_timestamp);

    printf("%s", bbuf);

    // // save the entity body in the cached file
    // fwrite(bbuf, sizeof(bbuf[0]), strlen(bbuf), cached_file);

  // }

  return 0;
}
