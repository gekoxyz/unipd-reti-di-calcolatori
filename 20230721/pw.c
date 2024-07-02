#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <sys/types.h>
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

int pid;
struct sockaddr_in server_addr, remote, server;
char request[10000];
char request2[10000];
char response[1000];
char response2[10000];

struct header {
  char *name;
  char *value;
};

struct header request_headers[100];
struct header response_headers[100];

struct hostent *he;

int main() {
  char hbuffer[10000];
  char buffer[2000];
  char *reqline;
  char *scheme, *hostname, *port;
  char *filename;
  FILE *fin;
  int c;
  int n;
  int i, j, t, sockfd, destinationfd;
  int yes = 1;
  int len;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
    printf("errno = %d\n", errno);
    perror("Socket Fallita");
    return -1;
  }
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = 0;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR) {
    perror("setsockopt fallita");
    return 1;
  }

  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
    perror("Bind Fallita");
    return -1;
  }

  if (listen(sockfd, 10) == SOCKET_ERROR) {
    perror("Listen Fallita");
    return -1;
  }

  printf("Server running on port %d\n", SERVER_PORT);

  remote.sin_family = AF_INET;
  remote.sin_port = htons(0);
  remote.sin_addr.s_addr = 0;

  while (1) {
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    int clientfd = accept(sockfd, (struct sockaddr *)&remote, &client_addr_size);
    printf("Remote address: %.8X\n", remote.sin_addr.s_addr);

    if (fork() == 0) {
      if (clientfd == SOCKET_ERROR) {
        perror("Accept fallita");
        exit(1);
      }

      // empty hbuffer and headers
      bzero(hbuffer, 10000);
      bzero(request_headers, 100 * sizeof(struct header));

      // process the HTTP request to read all the headers and remove CRLFs
      reqline = request_headers[0].name = hbuffer;
      for (i = 0, j = 0; read(clientfd, hbuffer + i, 1); i++) {
        printf("%c", hbuffer[i]);
        if (hbuffer[i] == '\n' && hbuffer[i - 1] == '\r') {
          hbuffer[i - 1] = 0;  // Termino il token attuale
          if (!request_headers[j].name[0]) break;
          request_headers[++j].name = hbuffer + i + 1;
        }
        if (hbuffer[i] == ':' && !request_headers[j].value && j > 0) {
          hbuffer[i] = 0;
          request_headers[j].value = hbuffer + i + 1;
        }
      }

      printf("Request line: %s\n", reqline);
      char *method = reqline;
      for (i = 0; i < 100 && reqline[i] != ' '; i++);
      reqline[i++] = 0;  // add the 0 to end the string in the request line
      char *url = reqline + i;
      for (; i < 100 && reqline[i] != ' '; i++);
      reqline[i++] = 0;
      char *ver = reqline + i;
      for (; i < 100 && reqline[i] != '\r'; i++);
      reqline[i++] = 0;

      /*
      The proxy has 2 different working modes:
      - standard connection, with the GET method:
        - parse the URL
        - get the server name
        - gethostbyname to resolve the address
        - send the request
        - get the response and send it to the client
      - tunnel connection, with the CONNECT method:
        - parse the connect to find the hostname
        – gethostbyname to resolve the address
        - generate two children processes to work in parallel
      */
      if (!strcmp(method, "GET")) {
        printf(ANSI_COLOR_GREEN "FOUND GET" ANSI_COLOR_RESET "\n");
        // process the URL
        scheme = url;
        printf("url=%s\n", url);
        // http:
        for (i = 0; url[i] != ':' && url[i]; i++);
        if (url[i] == ':')
          url[i++] = 0;
        else {
          printf("Parse error, expected ':'");
          exit(1);
        }
        // http://
        if (url[i] != '/' || url[i + 1] != '/') {
          printf("Parse error, expected '//'");
          exit(1);
        }
        i = i + 2;           // skip the //
        hostname = url + i;  // hostname starts after http://
        for (; url[i] != '/' && url[i]; i++);
        if (url[i] == '/')
          url[i++] = 0;  // put string terminator after next / to terminate hostname
        else {
          printf("Parse error, expected '/'");
          exit(1);
        }

        filename = url + i;
        printf("Schema: %s, hostname: %s, filename: %s\n", scheme, hostname, filename);
        // start the connection to the address requested by the client
        he = gethostbyname(hostname);
        printf("%d.%d.%d.%d\n", (unsigned char)he->h_addr[0], (unsigned char)he->h_addr[1], (unsigned char)he->h_addr[2], (unsigned char)he->h_addr[3]);
        if ((destinationfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
          printf("errno = %d\n", errno);
          perror("Socket Fallita");
          exit(-1);
        }
        server.sin_family = AF_INET;
        server.sin_port = htons(80);
        server.sin_addr.s_addr = *(unsigned int *)(he->h_addr);
        if (connect(destinationfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
          perror("Connect Fallita");
          exit(1);
        }
        // make HTTP request to the host
        sprintf(request, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", filename, hostname);
        printf("%s\n", request);
        write(destinationfd, request, strlen(request));

        char response_header[2000] = {0}; // this will be used if i don't have a 301
        int response_header_size = 0;

        // if the response is 301 resolve it and send it to the client
        bzero(buffer, 2000);
        bzero(response_headers, 100 * sizeof(struct header));
        response_headers[0].name = buffer;
        // parsing the HTTP request to read all the headers and remove CRLFs
        for (j = 0, i = 0; read(destinationfd, buffer + i, 1); i++) {
          response_header[i] = buffer[i];
          response_header_size++;
          if (buffer[i] == '\n' && buffer[i - 1] == '\r') {
            buffer[i - 1] = 0;
            if (!response_headers[j].name[0]) break;
            response_headers[++j].name = &buffer[i + 1];
          }
          if ((buffer[i] == ':') && (response_headers[j].value == NULL)) {
            buffer[i] = 0;
            response_headers[j].value = &buffer[i + 2];
            // printf(ANSI_COLOR_RED "TROVO ROBA" ANSI_COLOR_RESET);
          }
        }

        // for (i = 0; i < j; i++) {
        //   printf("%s --> %s\n", response_headers[i].name, response_headers[i].value);
        // }

        if (strcmp(response_headers[0].name, "HTTP/1.1 301 Moved Permanently") == 0) {
          printf(ANSI_COLOR_YELLOW "Found a 301 redirect\n" ANSI_COLOR_RESET);

          int location_header_index = -1;
          for (i = 0; i < j; i++) {
            if (strcmp(response_headers[i].name, "Location") == 0) {
              location_header_index = i;
            }
          }

          char *location = response_headers[location_header_index].value;
          char *new_filename = filename;
          char *new_hostname = hostname;

          // check that Location doesn't contain http:// or https:// (it's not a new url)
          if (strncmp(location, "http://", 7) == 0) {
            for (int k = 7; k < 100; k++) {
              if (location[k] == '/') {
                location[k] = 0;
                new_hostname = location;
                new_filename = &location[k + 1];
              }
            }
          } else if (strncmp(location, "https://", 8) == 0) {
            for (int k = 8; k < 100; k++) {
              if (location[k] == '/') {
                location[k] = 0;
                new_hostname = location;
                new_filename = &location[k + 1];
              }
            }
          } else {
            new_filename = location;
          }

          printf("*******************\n");
          printf("new_hostname --> %s\n", new_hostname);
          printf("new_filename --> %s\n", new_filename);
          printf("*******************\n");

          he = gethostbyname(new_hostname);
          if ((destinationfd = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
            printf("errno = %d\n", errno);
            perror("Socket Fallita");
            exit(-1);
          }
          server.sin_family = AF_INET;
          server.sin_port = htons(80);
          server.sin_addr.s_addr = *(unsigned int *)(he->h_addr);
          if (connect(destinationfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
            perror("Connect Fallita");
            exit(1);
          }

          sprintf(request, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", new_filename, new_hostname);
          write(destinationfd, request, strlen(request));
          printf("*******************\n");
          printf(ANSI_COLOR_MAGENTA "%s\n" ANSI_COLOR_RESET, request);
          printf("*******************\n");

          char body_buffer[2000] = {0};
          // write the response to the client
          while (t = read(destinationfd, body_buffer, 2000)) {
            write(clientfd, body_buffer, t);
          }
        } else {
          write(clientfd, response_header, response_header_size);

          // write the response to the client
          while (t = read(destinationfd, buffer, 2000)) {
            write(clientfd, buffer, t);
          }
        }
        close(destinationfd);


      } else if (!strcmp("CONNECT", method)) {  // it is a connect  host:port
        printf(ANSI_COLOR_GREEN "FOUND CONNECT" ANSI_COLOR_RESET "\n");
        hostname = url;
        for (i = 0; url[i] != ':'; i++);
        url[i] = 0;
        port = url + i + 1;
        printf("hostname:%s, port:%s\n", hostname, port);
        he = gethostbyname(hostname);
        if (he == NULL) {
          printf("Gethostbyname failed\n");
          return 1;
        }
        // start the connection to the address requsted by the client
        printf("Connecting to address = %u.%u.%u.%u\n", (unsigned char)he->h_addr[0], (unsigned char)he->h_addr[1], (unsigned char)he->h_addr[2], (unsigned char)he->h_addr[3]);
        destinationfd = socket(AF_INET, SOCK_STREAM, 0);
        if (destinationfd == SOCKET_ERROR) {
          perror("Socket to server failed");
          return 1;
        }
        server.sin_family = AF_INET;
        server.sin_port = htons((unsigned short)atoi(port));
        server.sin_addr.s_addr = *(unsigned int *)he->h_addr;
        if (connect(destinationfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
          perror("Connect to server failed");
          exit(0);
        }
        sprintf(response, "HTTP/1.1 200 Established\r\n\r\n");
        write(clientfd, response, strlen(response));
        // check if the code is running on the chlild or the parent
        if (!(pid = fork())) {
          // child -> read data from the client and write it as request
          while (t = read(clientfd, request2, 2000)) {
            write(destinationfd, request2, t);
          }
          exit(0);
        } else {
          // parent -> read data from the destination and send it to the client
          while (t = read(destinationfd, response2, 2000)) {
            write(clientfd, response2, t);
          }
          kill(pid, SIGTERM);
          close(destinationfd);
        }
      } else {
        printf("The server does not support the functionality required to fulfill the request");
        sprintf(response, "HTTP/1.1 501 Not Implemented\r\n\r\n");
        write(clientfd, response, strlen(response));
      }
      close(clientfd);
      exit(1);
    } else {
      close(clientfd);
      continue;
    }
  }
  close(sockfd);
}
