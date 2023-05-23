#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <string.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include "client.h"

#define HOST "34.254.242.81"
#define PORT 8080
#define TYPE "application/json"
#define TOKEN "Authorization: Bearer "
#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define ACCESS "/api/v1/tema/library/access"
#define BOOKS "/api/v1/tema/library/books"
#define LOGOUT "/api/v1/tema/auth/logout"

char *serialize_user(struct User user) {
	JSON_Value *value = json_value_init_object();
	JSON_Object *object = json_value_get_object(value);
	json_object_set_string(object, "username", user.username);
	json_object_set_string(object, "password", user.password);
	return json_serialize_to_string(value);
}

struct User init_user(struct User user) {
	user.username = calloc(LINELEN, sizeof(char));
	user.password = calloc(LINELEN, sizeof(char));
	user.cookie = NULL;
	user.token = NULL;

	 // Primim username-ul de la utilizator.
	fprintf(stdout, "username=");
	fgets(user.username, LINELEN, stdin);
	strtok(user.username, "\n"); // Remove trailing newline

	char *spaces_check = user.username;

	strtok(spaces_check, "' '");
	if (strlen(spaces_check) != strlen(user.username)) {
		fprintf(stderr, "Spaces are not allowed in username, try logging in again.\n");
		return user;
	}

	// Primim parola de la utilizator.
	fprintf(stdout, "password=");
	fgets(user.password, LINELEN, stdin);
	strtok(user.password, "\n"); // Remove trailing newline

	spaces_check = user.password;

	strtok(spaces_check, "' '");
	if (strlen(spaces_check) != strlen(user.password)) {
		fprintf(stderr, "Spaces are not allowed in password, try logging in again.\n");
		return user;
	}

	user.serialized_value = serialize_user(user);
	return user;
}

char *serialize_book(struct Book book) {
	char page_count_string[BUFLEN];
	sprintf(page_count_string, "%d", book.page_count);
	// pot sa fac un fel de Factory din java cu book si user separat
	JSON_Value *value = json_value_init_object();
	JSON_Object *object = json_value_get_object(value);
	json_object_set_string(object, "title", book.title);
	json_object_set_string(object, "author", book.author);
	json_object_set_string(object, "genre", book.genre);
	json_object_set_string(object, "page_count", page_count_string);
	json_object_set_string(object, "publisher", book.publisher);
	return json_serialize_to_string(value);
}

struct Book init_book(struct Book book, int *error) {
	book.title = calloc(LINELEN, sizeof(char));
	book.author = calloc(LINELEN, sizeof(char));
	book.genre = calloc(LINELEN, sizeof(char));
	book.publisher = calloc(LINELEN, sizeof(char));

	printf("title=");
	char buffer[LINELEN];
	memset (buffer, 0, LINELEN);
	fgets (buffer, LINELEN, stdin);

	if (strlen(buffer) != 0) {

		if (buffer[strlen (buffer) - 1] == '\n'){
			buffer[strlen (buffer) - 1] = '\0';
		}
		strcpy(book.title, buffer);

	} else {
		fprintf(stderr, "Error: Incorrect field. Please try again.\n");
		*error = 1;
		return book;
	}

	printf("author=");
	memset (buffer, 0, LINELEN);
	fgets (buffer, LINELEN, stdin);

	if (strlen(buffer) != 0) {
		if (buffer[strlen (buffer) - 1] == '\n'){
			buffer[strlen (buffer) - 1] = '\0';
		}
		strcpy(book.author, buffer);
		
	} else {
		fprintf(stderr, "Error: Incorrect field. Please try again.\n");
		*error = 1;
		return book;
	}
	
	printf("genre=");
	memset (buffer, 0, LINELEN);
	fgets (buffer, LINELEN, stdin);

	if (strlen(buffer) != 0) {
		if (buffer[strlen (buffer) - 1] == '\n'){
			buffer[strlen (buffer) - 1] = '\0';
		}
		strcpy(book.genre, buffer);
		
	} else {
		fprintf(stderr, "Error: Incorrect field. Please try again.\n");
		*error = 1;
		return book;
	}
	
	printf("publisher=");
	memset (buffer, 0, LINELEN);
	fgets (buffer, LINELEN, stdin);

	if (strlen(buffer) != 0) {
		if (buffer[strlen (buffer) - 1] == '\n'){
			buffer[strlen (buffer) - 1] = '\0';
		}
		strcpy(book.publisher, buffer);
		
	} else {
		fprintf(stderr, "Error: Incorrect field. Please try again.\n");
		*error = 1;
		return book;
	}

	printf("page_count=");
	memset (buffer, 0, LINELEN);
	fgets (buffer, LINELEN, stdin);

	if (strlen(buffer) != 0) {

		// Eliminam endline-ul de la finalul sirului citit de la tastatura.
		if (buffer[strlen (buffer) - 1] == '\n'){
			buffer[strlen (buffer) - 1] = '\0';
		}

		int valid = 1;
		int len = strlen (buffer);
		
		for (int i = 0; i < len && valid == 1; i++){
			if (buffer[i] < '0' || buffer[i] > '9'){
				valid = 0;
				break;
			}
		}

		if (valid == 0 || len == 0){
			fprintf(stderr, "Error: Incorrect field. Please try again.\n");
			*error = 1;
			return book;
		} else {
			book.page_count = atoi(buffer);
		}
	} else {
		fprintf(stderr, "Error: Incorrect field. Please try again.\n");
		*error = 1;
		return book;
	}

	book.serialized_value = serialize_book(book);
	return book;
}


// Functia primeste cookie-ul de logare si tokenul JWT de autentificare la
// biblioteca, iar apoi intoarce un sir de 2 string-uri care reprezinta
// headerele ce trebuie adaugate in requestul de HTTP, pe langa cele standard.
char** init_header(char* cookie, char* token, int* cookie_count) {
    char** cookies;
    char* auth_header;

    // Calculate the initial cookie count based on the presence of cookie and token
    *cookie_count = (cookie != NULL) + (token != NULL);

    // Allocate memory for the array of cookie headers
    cookies = calloc(*cookie_count, sizeof(char*));
    if (cookies == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return NULL;
    }

    if (cookie != NULL) {
        // Allocate memory for the cookie header and check for allocation failure
        size_t cookie_length = strlen(cookie) + strlen("Cookie: ") + 1;
        cookies[0] = malloc(cookie_length * sizeof(char));
        if (cookies[0] == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            free(cookies);
            return NULL;
        }

        snprintf(cookies[0], cookie_length, "Cookie: %s", cookie);  // Construct the cookie header
    }

    if (token != NULL) {
        // Allocate memory for the authentication header and check for allocation failure
        size_t auth_header_length = strlen(TOKEN) + strlen(token) + 1;
        auth_header = malloc(auth_header_length * sizeof(char));
        if (auth_header == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            free(cookies[0]);
            free(cookies);
            return NULL;
        }

        snprintf(auth_header, auth_header_length, "%s%s", TOKEN, token);  // Construct the authentication header
        cookies[*cookie_count - 1] = auth_header;  // Add the authentication header to the cookies array
    }

    return cookies;  // Return the array of cookie headers
}



int main(int argc, char *argv[])
{
	char buffer[LINELEN];
	struct User user;
	user.cookie = NULL;
	user.token = NULL;

	setvbuf (stdout, NULL, _IONBF, 0);

	while (1){
		fgets (buffer, LINELEN, stdin);

		int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

		if (strncmp(buffer, "register", 8) == 0){
			if (user.cookie != NULL){
				fprintf (stderr, "Error: You are currently logged in. Please logout in order to register another account.\n");
			} else {
				char *message;
				char *response;

				user = init_user(user);

				// Cream mesajul de tip POST de care avem nevoie si il trimitem apoi la
				// server.
				message = compute_post_request (HOST, REGISTER, TYPE,
												user.serialized_value, NULL, 0);
				send_to_server (sockfd, message);
				response = receive_from_server (sockfd);

				char *ret = strstr(response, "error");
				if (ret == NULL) {
					printf("You have successfully registered!\n");
				} else {
					fprintf(stderr, "Registeration failed. Please try again.\n");
				}
			}

			close_connection (sockfd);
		}
		else if (strncmp(buffer, "login", 5) == 0){
			// Daca utilizatorul era deja logat, atunci il anuntam.
			if (user.cookie != NULL){
				fprintf (stderr, "Error: You are already logged in.\n");
			} else {
				user = init_user(user);
				char *message;
				char *response;
				// Cream mesajul de tip POST de care avem nevoie si il trimitem apoi la
				// server.
				message = compute_post_request (HOST, LOGIN, TYPE,
												user.serialized_value, NULL, 0);
				send_to_server (sockfd, message);
				response = receive_from_server (sockfd);

				char *ret = strstr(response, "error");

				// Daca nu s-a reusit logarea ne oprim.
				if (ret == NULL) {
					char *cookie = strstr(response, "Set-Cookie: ");
					strtok(cookie, "\r");
					char *cookie_auth_copy = malloc(strlen(cookie) * sizeof(char) - 12 * sizeof(char));
					memcpy(cookie_auth_copy, cookie + 12, strlen(cookie) - 12 * sizeof(char));
					user.cookie = cookie_auth_copy;
					printf("You have successfully logged in!\n");
				} else {
					fprintf(stderr, "Error: Login failed. Please try again.\n");
				}
					close_connection (sockfd);
			}
		}
		else if (strncmp(buffer, "enter_library", 13) == 0){
			if (user.cookie != NULL) {
				char *message;
				char *response = NULL;
				char **cookies;
				int cookie_count = 0;

				cookies = init_header (user.cookie, NULL, &cookie_count);

				if (cookies == NULL){
						fprintf(stderr, "Error: Unable to generate headers.\n");
				} else {
					message = compute_get_request (HOST, ACCESS, NULL, cookies, cookie_count);
					send_to_server (sockfd, message);
					response = receive_from_server (sockfd);
					char *ret = strstr(response, "error");

					// Daca nu s-a reusit logarea ne oprim.
					if (ret == NULL) {
						char *tok = strstr(response, "token");
						char *tokk = malloc(strlen(tok) - 8);
						strcpy(tokk, tok + 8);
						tokk[strlen(tokk) - 2] = '\0';
						user.token = tokk;
						printf("Access to library granted!\n");
					} else {
						fprintf(stderr, "Error: Library unavailable. Please try again.\n");
					}
				}
			} else {
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}

			close_connection (sockfd);
		}
		else if (strncmp(buffer, "get_books", 9) == 0){
			if (user.cookie != NULL) {
				if (user.token != NULL) {
					char *message;
					char *response;
					char **cookies;
					int cookie_count = 0;

					cookies = init_header (user.cookie, user.token, &cookie_count);

					// Daca nu am putut obtine headere-le pentru token si cookie, atunci nu putem
					// trimite mai departe requestul.
					if (cookies == NULL){
						fprintf(stderr, "Error: Unable to generate headers.\n");
					} else {
						message = compute_get_request (HOST, BOOKS, NULL, cookies, 
													cookie_count);
						send_to_server (sockfd, message);
						response = receive_from_server (sockfd);
						char *ret = strstr(response, "error");

						if (ret == NULL) {
							JSON_Value* root_value = NULL;
							JSON_Array* books_array = NULL;
							char* json_response = extract_json_array(response);

							root_value = json_parse_string(json_response);
							books_array = json_value_get_array(root_value);

							size_t book_count = json_array_get_count(books_array);
							if (book_count == 0) {
								printf("Error: There are no books in the library yet.\n");
							} else {
								for (size_t i = 0; i < book_count; i++) {
									JSON_Object* book = json_array_get_object(books_array, i);
									const char* title = json_object_get_string(book, "title");
									int id = (int)json_object_get_number(book, "id");
									printf("title: %s and id: %d\n", title, id);
								}
							}
							json_value_free(root_value);
						} else {
							fprintf(stderr, "Error: Library unavailable. Please try again.\n");
						}
					}
				} else {
					fprintf(stderr, "Error: You do not have access to the library!\n");
				}
			} else {
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}
			close_connection (sockfd);
		}
		else if (strncmp(buffer, "get_book", 8) == 0){
			if (user.cookie != NULL) {
				if (user.token != NULL) {
					char* message;
					char* response;
					char id[LINELEN];
					char** cookies;
					int cookie_count = 0;

					printf("id=");
					fgets(id, LINELEN, stdin);

					if (id[strlen(id) - 1] == '\n') {
						id[strlen(id) - 1] = '\0';
					}

					int valid = 1;
					for (int i = 0; i < strlen(id); i++) {
						if (id[i] < '0' || id[i] > '9') {
							valid = 0;
							break;
						}
					}

					if (valid) {
						size_t final_url_length = strlen(id) + strlen(BOOKS) + 2;
						char* final_url = malloc(final_url_length * sizeof(char));
						if (final_url == NULL) {
							fprintf(stderr, "Error: Failed to allocate memory.\n");
						} else {
							snprintf(final_url, final_url_length, "%s/%s", BOOKS, id);

							// Generate headers to be added to the request.
							cookies = init_header(user.cookie, user.token, &cookie_count);
							if (cookies == NULL) {
								fprintf(stderr, "Error: Unable to generate headers.\n");
							} else {
								message = compute_get_request(HOST, final_url, NULL, cookies, cookie_count);
								send_to_server(sockfd, message);
								response = receive_from_server(sockfd);
								char* ret = strstr(response, "error");

								if (ret == NULL) {
									JSON_Value* root_value;
									JSON_Object* book;
									char* json_response = basic_extract_json_response(response);
									if (json_response != NULL) {
										root_value = json_parse_string(json_response);
										book = json_value_get_object(root_value);

										// Extract and display the book details
										const char* title = json_object_get_string(book, "title");
										const char* author = json_object_get_string(book, "author");
										const char* genre = json_object_get_string(book, "genre");
										const char* publisher = json_object_get_string(book, "publisher");
										int page_count = (int)json_object_get_number(book, "page_count");

										printf("title = %s\n", title);
										printf("author = %s\n", author);
										printf("genre = %s\n", genre);
										printf("publisher = %s\n", publisher);
										printf("page_count = %d\n", page_count);

										json_value_free(root_value);
										free(final_url);
									} else {
										fprintf(stderr, "Error: Invalid response.\n");
									}
								} else {
									fprintf(stderr, "Error: No book was found!\n");
								}
							}
						}
					} else {
						fprintf(stderr, "Error: ID is not a number. Please try again.\n");
					}
				} else {
					fprintf(stderr, "Error: You do not have access to the library!\n");
				}
			} else {
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}
			close_connection (sockfd);
		}
		else if (strncmp(buffer, "add_book", 8) == 0){
			if (user.cookie != NULL) {
				if (user.token != NULL) {
					int error = 0;
					struct Book book = init_book(book, &error);
					char *message;
					char *response;
					char **cookies;
					int cookie_count = 0;
					if (error == 0) {
						cookies = init_header (user.cookie, user.token, &cookie_count);
						if (cookies == NULL){
							fprintf(stderr, "Error: Unable to generate headers.\n");
						} else {
							message = compute_post_request (HOST, BOOKS, TYPE, 
															book.serialized_value, cookies, cookie_count);
							send_to_server (sockfd, message);
							response = receive_from_server (sockfd);
							char* ret = strstr(response, "error");

							if (ret != NULL) {
								fprintf(stderr, "Error: %s\n", ret + 7);
							}

							printf("You have succsessfully added %s to the library!\n", book.title);
						}
					} else {
						fprintf (stderr, "Error: The information was wrong or incomplete. Please try again.\n");
					}
				} else {
					fprintf(stderr, "Error: You do not have access to the library!\n");
				}
			} else {
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}
			close_connection (sockfd);
		}
		else if (strncmp(buffer, "delete_book", 11) == 0){
			if (user.cookie != NULL) {
				if (user.token != NULL) {
					char* message;
					char* response;
					char** cookies;
					char id[LINELEN];
					int cookie_count = 0;

					printf("id=");
					fgets(id, LINELEN, stdin);

					if (id[strlen(id) - 1] == '\n') {
						id[strlen(id) - 1] = '\0';
					}

					// Check if id contains only digits
					int is_valid_id = 1;
					for (int i = 0; i < strlen(id); i++) {
						if (id[i] < '0' || id[i] > '9') {
							is_valid_id = 0;
							break;
						}
					}

					if (!is_valid_id) {
						fprintf(stderr, "Error: Id is a number, please try again.\n");
					} else {
						cookies = init_header(user.cookie, user.token, &cookie_count);
						if (cookies == NULL) {
							fprintf(stderr, "Error: Unable to generate headers.\n");
						} else {
							char* final_url = malloc(strlen(BOOKS) + strlen(id) + 2);
							if (final_url == NULL) {
								fprintf(stderr, "Error: There was an error allocating memory.\n");
							} else {
								sprintf(final_url, "%s/%s", BOOKS, id);

								message = compute_delete_request(HOST, final_url, cookies, cookie_count);
								send_to_server(sockfd, message);
								response = receive_from_server(sockfd);
								char* ret = strstr(response, "error");

								if (ret != NULL) {
									fprintf(stderr, "Error: %s\n", ret + 7);
								} else {
									printf("Book with id %s has been successfully deleted from the library!\n", id);
								}

								free(final_url);
							}
						}
					}

				} else {
					fprintf(stderr, "Error: You do not have access to the library!\n");
				}
			} else {
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}
			close_connection (sockfd);
		}
		else if (strncmp(buffer, "logout", 6) == 0){
			if (user.cookie != NULL) {
				char *message;
				char *response;
				char **cookies;
				int cookie_count;

				cookies = init_header (user.cookie, user.token, &cookie_count);

				message = compute_get_request (HOST, LOGOUT, NULL, cookies, cookie_count);
				send_to_server (sockfd, message);
				response = receive_from_server (sockfd);
				char* ret = strstr(response, "error");

				if (ret != NULL) {
					fprintf(stderr, "Error: %s\n", ret + 7);
				}

				user.cookie = NULL;
				user.token = NULL;
				printf("Logout successful!\n");
				close_connection (sockfd);
			} else {
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}
		}
		else if (strncmp(buffer, "exit", 4) == 0){
			
			close_connection(sockfd);
			break;
		}
		else{
			fprintf (stderr, "Error: Please enter a valid command.\n");
		}
	}

	return 0;
}
