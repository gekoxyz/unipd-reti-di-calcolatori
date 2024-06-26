#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_ERROR -1
#define SERVER_PORT 80

int main() {
  int try, i;
  char response[2000000];
  struct sockaddr_in server;

  // creating sockfd, with:
  // socket family AF_INET ipv4 sockets
  // socket type SOCK_STREAM connection oriented sockets
  // 0 is the default protocol for AF_INET and SOCK_STREAM (TCP)
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == SOCKET_ERROR) {
    printf("Errno = %d (%d)\n", errno, EAFNOSUPPORT);
    perror("Socket fallita:");
    return 1;
  }
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);
  // setting the ip to 142.250.187.196
  unsigned char *ip = (unsigned char *)&server.sin_addr.s_addr;
  ip[0] = 142;
  ip[1] = 250;
  ip[2] = 187;
  ip[3] = 196;
  // try to connect to the socket
  try = connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
  if (try == SOCKET_ERROR) {
    perror("Connect fallita");
    return 1;
  }
  // write the request on the socket
  write(sockfd, "GET / HTTP/1.0\r\n\r\n", 18);
  // wait for the response to be written
  sleep(1);
  // print the response
  for (i = 0; (try = read(sockfd, response + i, 1999999 - i)); i += try)
    printf("%s\n", response);
  return 0;
}
