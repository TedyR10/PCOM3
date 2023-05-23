#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET/DELETE request string
char* compute_get_or_delete_request(char* method, char* host, char* url, char* query_params,
                      char** cookies, int cookie_count);

// computes and returns a POST request string
char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
                           char **cookies, int cookies_count);
#endif
