#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SOCKET_ERROR                    (-1)
#define SERVER_PORT                     (8000)
#define BACKLOG_SIZE                    (5)

#define HTTP_VERSION                    "1.0"
#define REPONSE_STATUS_LINE(VERSION, STATUS_CODE) ("HTTP/" VERSION " " #STATUS_CODE "\r\n\r\n")
#define RESPONSE_STATUS_CODE_PATH(STATUS_CODE) ("status-code/" #STATUS_CODE ".html")

// TODO: Implement request parsing

enum respose_status_code {
    OK_REQUEST,
    BAD_REQUEST,
    FORBIDDEN_REQUEST,
    INVALID_URI,
    NOT_FOUND_REQUEST,
};

struct response_type_t {
    char *file_path;
    char *response;
    int status_code;
};

static struct response_type_t response[] = {
    [OK_REQUEST] = {
        .status_code = 200, 
        .file_path = "",
        .response = REPONSE_STATUS_LINE(HTTP_VERSION, 200),
    },
    [BAD_REQUEST] = {
        .status_code = 400,
        .file_path = "",
        .response = REPONSE_STATUS_LINE(HTTP_VERSION, 400),
    },
    [FORBIDDEN_REQUEST] = {
        .status_code = 403,
        .file_path = "",
        .response = REPONSE_STATUS_LINE(HTTP_VERSION, 403),
    },
    [NOT_FOUND_REQUEST] = {
        .status_code = 404,
        .file_path = RESPONSE_STATUS_CODE_PATH(404),
        .response = REPONSE_STATUS_LINE(HTTP_VERSION, 404),
    }
};

enum respose_status_code parse_request(int sockfd);
void return_content(const char *path, int sockfd);

int main(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return 1;
    }

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
    };

    if (bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return 1;
    }

    if (listen(sockfd, BACKLOG_SIZE) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return 1;
    }

    while (1) {
        struct sockaddr_in caddr;
        socklen_t caddr_size = sizeof(caddr);
        int clientfd = accept(sockfd, (struct sockaddr *)&caddr, &caddr_size);
        if (clientfd == SOCKET_ERROR) {
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
            return 1;
        }

        enum respose_status_code status_code = parse_request(clientfd);
        write(clientfd, response[status_code].response, strlen(response[status_code].response));
        return_content(response[status_code].file_path, clientfd);

        //ssize_t request_size = read(clientfd, request, MAX_REQUEST_SIZE);
        //if (request_size == SOCKET_ERROR) {
        //    fprintf(stderr, "ERROR: %s\n", strerror(errno));
        //    return 1;
        //}
        //printf("----- Request -----\n%s-------------------\n", request);
        close(clientfd);
    }

    return 0;
}

/*
 * E.g:
 *    GET / HTTP/1.0\r\n
 *    Host:127.0.0.1:8000\r\n\r\n
 *
 * --- CFG OF REQUEST ---
 * <REQUEST>        -> <STATUS LINE> <HEADER FIELD>* <TERMINATOR>
 * <STATUS LINE>    -> <METHOD> ' ' <URI> ' ' 'HTTP/' ('1.0' | '1.1') <TERMINATOR>
 * <HEADER FIELD>   -> <NAME> ':' <BLANK>? <VALUE> <TERMINATOR>
 * <NAME>           -> <String of various symbols>
 * <VALUE>          -> <String of various symbols>
 * <METHOD>         -> 'GET' | 'POST'
 * <URI>            -> <string without spaces, path separated by '/'>
 * <BLANK>          -> ' ' | '\t'
 * <TERMINATOR>     -> '\r\n'
 */
typedef enum end_t {
    NOT_END,
    END_LINE,
    END_REQUEST,
} end_t;

end_t handle_end_line(int sockfd, char *pos, end_t end) {
    char next;
    read(sockfd, &next, 1);                         
    if (next != '\n') {                             
        return end;
    }                                               
    *pos = '\0';                              
    switch (end) {
        case NOT_END:
            return END_LINE;
        case END_LINE:
            return END_REQUEST;
        case END_REQUEST:
            return END_REQUEST;
    }
}

enum respose_status_code parse_request(int sockfd) {
#define MAX_REQUEST_SIZE                (2048)
#define MAX_STATUS_LINE_LENGTH          (256)
#define HANDLE_END_LINE(SOCK, POS, END)                 \
    do {                                                \
        char next;                                      \
    } while (0)

    char status_line[MAX_STATUS_LINE_LENGTH + 1] = {0};
    char buffer[MAX_REQUEST_SIZE + 1] = {0};
    char c = 0;
    end_t end = NOT_END;
    int fields = 0;

    {   // PARSE STATUS LINE
        for (int i = 0; i < MAX_STATUS_LINE_LENGTH && read(sockfd, &c, 1) && end != END_REQUEST; ++i) {
            read(sockfd, &c, 1);
            switch (c) {
                case '\r':
                    end = handle_end_line(sockfd, &buffer[i], end);
                    break;
                default:
                    status_line[i] = c;
            }
            if (end) break;
        }
    }
parse_header:
    { // PARSE HEADER FIELD
        for (int i = 0; i < MAX_REQUEST_SIZE && read(sockfd, &c, 1) && end != END_REQUEST; ++i) {
            switch (c) {
                case '\r':
                    end = handle_end_line(sockfd, &buffer[i], end);
                    break;
                default:
                    status_line[i] = c;
            }
        }
    }
    printf("%s\n", status_line);
    return NOT_FOUND_REQUEST;
#undef MAX_STATUS_LINE_LENGTH
}

void return_content(const char *path, int sockfd) {
    FILE *file = fopen(path, "r");
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char data[size];
    fread(data, sizeof(*data), size, file);
    fclose(file);

    write(sockfd, data, size);
}
