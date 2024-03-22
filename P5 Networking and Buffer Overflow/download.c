#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#define MAXLINE 15000

int main(int argc, char *argv[]) { //[0] program name, [1] server, [2] port, [3] remote file, [4] output file
  /* 1 - parse command line */
  // check argument count
  if(argc != 5) {
  fprintf(stderr, "Incorrect usage, only %d args", argc);
	exit(1);
  }

  // extract args
  char *server, *port, *remote, *output;
  server = argv[1];
  port = argv[2];
  remote = argv[3];
  output = argv[4];

  // extract port
  /*
  char *end;
  long int portNum = strtol(argv[2], &end, 10);
  if(!(argv[2][0] != '\0' && end[0] == '\0')) {
	fprintf(stderr, "Invalid port '%s' (must be decimal)\n", argv[1]);
	exit(1);
  }
  */

  // check argument constraints
  if(server == NULL || remote == NULL || output == NULL) {
    fprintf(stderr, "Incorrect usage");
    exit(1);
  }
  
  int portNum = atoi(port);
  if (!(portNum >= 1) || !(portNum <= 65535)) {
    fprintf(stderr, "Incorrect usage");
    exit(1);
  }
  
  

  /* 2 - perform name resolution */
  //    node -> server; service -> port num as string; hints -> hints data structure should set ai_family to AF_INET
  //                                                            and ai_socktype to SOCK_STREAM to avoid getting IPv6 addresses
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  memset(&res, 0, sizeof(res));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int a = getaddrinfo(server, port, &hints, &res);
  if (a != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(a));
    exit(2);
  }

  /* 3 - connect to server */
  /* [create socket], construct request, connect to server at port, and send it */
  // socket
  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    fprintf(stderr, "socket: %s\n", strerror(errno));
    exit(3);
  }

  // connection
  //    1st param -> file descriptor that socket() returns
  int c = connect(sfd, res->ai_addr, res->ai_addrlen);
  if (c == -1) {
    fprintf(stderr, "connect: %s\n", strerror(errno));
    exit(3);
  }

  // construct request
  //    GET / HTTP/1.1\r\n
  //    Host: www.example.com\r\n
  //    \r\n
  char buff_out[MAXLINE];
  //fprintf(buff_out, MAXLINE, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", argv[1]);
  sprintf(buff_out, "GET / HTTP/1.1\r\nHost: %s:%s\r\n\r\n", argv[1], argv[2]);

  // send & read response from server
  // write
  //    1st param -> file descriptor
  int w = write(sfd, buff_out, strlen(buff_out));
  while (w == -1 && strcmp(strerror(errno), "EINTR") == 0 || w != strlen(buff_out)) { //if either write or read fail with an error code of EINTR, retry the operation again
    w = write(sfd, buff_out, strlen(buff_out));
  }
  if (w == -1) {
    fprintf(stderr, "write: %s\n", strerror(errno));
    exit(4);
  }

  // send
  /*
  if (send(sfd, &buff_out, strlen(buff_out)+1, 0) == -1) {
    fprintf(stderr, "send: %s\n", strerror(errno));
    exit(4);
  }
  */

  // read
  /**** 0123456789012345678 */
  //    HTTP/1.0 200 OK\r\n
  /**** 012345678901234567890 */
  //    Content-Length: 13\r\n
  //    Content-Type: text/html\r\n
  //    \r\n
  //    <html>\r\n
  //    <body>\r\n
  //    Hello, world!\r\n
  //    </body>\r\n
  //    </html>\r\n
  //    \r\n
  char buff_in[MAXLINE];
  int bytesRead = 0;
  int r = 0;
  while (bytesRead < MAXLINE) {
    r = read(sfd, &buff_in[bytesRead], MAXLINE - bytesRead);
    if (r == -1 && strcmp(strerror(errno), "EINTR") != 0) {
      fprintf(stderr, "read: %s\n", strerror(errno));
        exit(4);
    }
    if (r == 0) {
      break;
    }
    bytesRead += r;
  }

  // copy argv[4] into file name arrays
  // 123456789012
  // .html.header
  int outputLen = strlen(output);
  char *hfile, *bfile;
  outputLen += 13;
  hfile = (char*)calloc(outputLen, sizeof(char));
  outputLen -= 7;
  bfile = (char*)calloc(outputLen, sizeof(char));
  for (int i = 0; i < strlen(output); i++) {
    hfile[i] = argv[4][i];
    bfile[i] = argv[4][i];
  }
  char *h = ".html.header";
  char *b = ".html";
  strncat(hfile, h, 12);
  strncat(bfile, b, 5);

  // separate header and body, write to files
  FILE *header = fopen(hfile, "w+"); //arg[4].html.header
  FILE *body = fopen(bfile, "w+"); //arg[4].html
  if (!header) {
    fprintf(stderr, "fopen, header: %s\n", strerror(errno));
  }
  if (!body) {
    fprintf(stderr, "fopen, header: %s\n", strerror(errno));
  }
  // grab status code from buff_in[9,10,11]
  char statCode[4];
  for (int i = 9, j = 0; j < 4; i++, j++) {
    if (isdigit(buff_in[i])) {
      statCode[j] = buff_in[i];
    }
  }
  statCode[3] = '\0';
  
  // write header to file
  //    end of line: \r\n
  //    end of header: \r\n\r\n
  //
  /***** 01234567890123456 */
  //    "Content-Length: 3495"
  int headerCount, bodyCount;
  headerCount = bodyCount = 0;
  int flag = 0;

  while (flag < 1) {
    int i = 0;
    if (buff_in[i] == '\n') {
      fputs(&buff_in[i], header);
      headerCount++;
      i++;
      if (buff_in[i] == '\r') { //blank line reached
        //write \r
        fputs(&buff_in[i], header);
        headerCount++;
        i++;
        //write \n
        fputs(&buff_in[i], header);
        headerCount++;
        i++;
        //exit loop
        flag = 1;
      }
    } else {
      fputs(&buff_in[i], header);
      headerCount++;
      i++;
    }
  }
/*
  for (int i = 0; i < bytesRead; i++) {
    if (buff_in[i] == '\n') { //base case: EOL, check next char
      fputs(&buff_in[i], header);
      headerCount++;
      if (buff_in[i+1] == '\r') { //case 1: EOH
        fputs("\r\n", header);
        headerCount += 2;
        printf("right before break");
        break;
      } else { //case 2: not EOH, cont
        fputs(&buff_in[i], header);
        headerCount++;
      }
    } else {
      fputs(&buff_in[i], header);
      headerCount++;
    }
  }
  */
  // write body to file
  //    end of body: \r\n\r\n
  bodyCount = bytesRead - headerCount;
  for (int l = headerCount + 1; l < bodyCount - 1; l++) {
    fputs(&buff_in[l], body);
  }

  /* close connection and free memory */
  // close
  close(sfd);
  close(c);
  fclose(header);
  fclose(body);
  // free mem
  freeaddrinfo(res);
  free(hfile);
  free(bfile);
  
  int sc = atoi(statCode);
  printf("statCode = %i\n", sc);
  if (sc >= 200 && sc <= 299) {
    exit(0);
  } else {
    exit(5);
  }
}
// echo$? to check errors after program exits

// ASAN: clang -fsanitize=address -O1 -fno-omit-frame-pointer -g download.c -o d
//       ./d localhost 5678 /home.html output
// body: curl http://localhost:8591/
// header: curl -i http://localhost:8591/