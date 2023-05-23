#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char* compute_get_or_delete_request(char* method, char* host, char* url, char* query_params,
                      char** cookies, int cookie_count)
{
    char* message = calloc(BUFLEN, sizeof(char));
    char* line = calloc(LINELEN, sizeof(char));

    // Construct the request line based on the method and URL
    if (query_params != NULL) {
        sprintf(line, "%s %s?%s HTTP/1.1", method, url, query_params);
    } else {
        sprintf(line, "%s %s HTTP/1.1", method, url);
    }
    compute_message(message, line);

    // Add the Host header
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add additional headers, including cookies
    for (int i = 0; i < cookie_count; i++) {
        memset(line, 0, LINELEN);
        sprintf(line, "%s", cookies[i]);
        compute_message(message, line);
    }

    // Add a blank line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
                           char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Write the request type and add it to the message. Check if there are
    // query parameters and add them if present.
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Add the host name.
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add the payload type and the length of the content to be sent.
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    int len = strlen(body_data);
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %d", len);
    compute_message(message, line);

    // Add additional headers that need to be included in the request.
    // Typically, there will be a single header for cookies from the cookies array,
    // and the rest are additional headers that need to be added.
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            memset(line, 0, LINELEN);
            sprintf(line, "%s", cookies[i]);
            compute_message(message, line);
        }
    }

    // Add a line between headers and payload.
    compute_message(message, "");

    // Add the actual payload.
    memset(line, 0, LINELEN);
    compute_message(message, body_data);

    free(line);
    return message;
}
