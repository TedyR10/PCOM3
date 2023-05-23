#ifndef _CLIENT_
#define _CLIENT_

struct User {
	char *username;
	char *password;
	char *serialized_value;
    char *cookie;
    char *token;
};

struct Book {
	char *title;
	char *author;
	char *genre;
	char *publisher;
	int page_count;
	char *serialized_value;
};

#endif