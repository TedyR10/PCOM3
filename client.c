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

//Function that serializes the fields of user
char *serialize_user(struct User user) {
	JSON_Value *value = json_value_init_object();
	JSON_Object *object = json_value_get_object(value);
	json_object_set_string(object, "username", user.username);
	json_object_set_string(object, "password", user.password);

	char *serialized = json_serialize_to_string(value);
	json_value_free (value);
	return serialized;
}

//Function that initializes user
struct User init_user(struct User user) {
    user.username = calloc(LINELEN, sizeof(char));
    user.password = calloc(LINELEN, sizeof(char));
    user.cookie = NULL;
    user.token = NULL;

    // Receive the username from the user
    fprintf(stdout, "username=");
    fgets(user.username, LINELEN, stdin);
    strtok(user.username, "\n"); // Remove trailing newline

    char* spaces_check = user.username;

    strtok(spaces_check, "' '");
    if (strlen(spaces_check) != strlen(user.username)) {
        fprintf(stderr, "Spaces are not allowed in the username. Please try logging in again.\n");
        return user;
    }

    // Receive the password from the user
    fprintf(stdout, "password=");
    fgets(user.password, LINELEN, stdin);
    strtok(user.password, "\n"); // Remove trailing newline

    spaces_check = user.password;

    strtok(spaces_check, "' '");
    if (strlen(spaces_check) != strlen(user.password)) {
        fprintf(stderr, "Spaces are not allowed in the password. Please try logging in again.\n");
        return user;
    }

    user.serialized_value = serialize_user(user);
    return user;
}

//Function that serializes the fields of book
char* serialize_book(struct Book book) {
    char page_count_string[BUFLEN];
    sprintf(page_count_string, "%d", book.page_count);

    JSON_Value* value = json_value_init_object();
    JSON_Object* object = json_value_get_object(value);
    json_object_set_string(object, "title", book.title);
    json_object_set_string(object, "author", book.author);
    json_object_set_string(object, "genre", book.genre);
    json_object_set_string(object, "page_count", page_count_string);
    json_object_set_string(object, "publisher", book.publisher);

	char *serialized = json_serialize_to_string(value);
	json_value_free (value);
    return serialized;
}

//Function that initializes book
struct Book init_book(struct Book book, int* error) {
    // Allocate memory for book fields
    book.title = calloc(LINELEN, sizeof(char));
    book.author = calloc(LINELEN, sizeof(char));
    book.genre = calloc(LINELEN, sizeof(char));
    book.publisher = calloc(LINELEN, sizeof(char));

    // Read and validate the title
    printf("title=");
    char buffer[LINELEN];
    memset(buffer, 0, LINELEN);
    fgets(buffer, LINELEN, stdin);

    if (strlen(buffer) - 1 != 0) {
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        strcpy(book.title, buffer);
    } else {
        *error = 1;
    }

    // Read and validate the author
    printf("author=");
    memset(buffer, 0, LINELEN);
    fgets(buffer, LINELEN, stdin);

    if (strlen(buffer) - 1 != 0) {
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        strcpy(book.author, buffer);
    } else {
        *error = 1;
    }

    // Read and validate the genre
    printf("genre=");
    memset(buffer, 0, LINELEN);
    fgets(buffer, LINELEN, stdin);

    if (strlen(buffer) - 1 != 0) {
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        strcpy(book.genre, buffer);
    } else {
        *error = 1;
    }

    // Read and validate the publisher
    printf("publisher=");
    memset(buffer, 0, LINELEN);
    fgets(buffer, LINELEN, stdin);

    if (strlen(buffer) - 1 != 0) {
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        strcpy(book.publisher, buffer);
    } else {
        *error = 1;
    }

    // Read and validate the page count
    printf("page_count=");
    memset(buffer, 0, LINELEN);
    fgets(buffer, LINELEN, stdin);

    if (strlen(buffer) - 1 != 0) {
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        int valid = 1;
        int len = strlen(buffer);

        for (int i = 0; i < len && valid == 1; i++) {
            if (buffer[i] < '0' || buffer[i] > '9') {
                valid = 0;
                break;
            }
        }

        if (valid == 0 || len == 0) {
            *error = 1;
        } else {
            book.page_count = atoi(buffer);
        }
    } else {
        *error = 1;
    }

	// Serialize the book
	if (*error == 0)
    	book.serialized_value = serialize_book(book);
    return book;
}

//Function that creates the headers
char** init_headers(char* cookie, char* token, int* cookie_count) {
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
		// Read input from the user using fgets and store it in the buffer variable
		fgets(buffer, LINELEN, stdin);

		// Remove the trailing newline character from the buffer
		buffer[strlen(buffer) - 1] = '\0';

		// Open a connection to the server
		int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

		// Check if the user input is "register"
		if (strcmp(buffer, "register") == 0) {
			// Check if the user is already logged in
			if (user.cookie != NULL) {
				// Print an error message if the user is logged in
				fprintf(stderr, "Error: You are currently logged in. Please logout in order to register another account.\n");
			} else {
				char *message;
				char *response;
				// Initialize the user
				user = init_user(user);

				// Generate a POST request message with the necessary data and send it to the server
				message = compute_post_request(HOST, REGISTER, TYPE, user.serialized_value, NULL, 0);
				send_to_server(sockfd, message);

				// Receive the response from the server
				response = receive_from_server(sockfd);

				// Check if the response contains "error"
				char *ret = strstr(response, "error");
				if (ret == NULL) {
					// If the response does not contain "error", print a success message
					printf("You have successfully registered!\n");
				} else {
					// If the response contains "error", print a failure message
					fprintf(stderr, "Registration failed. Please try again.\n");

					//Free user in case of error
					free(user.username);
					free(user.password);
					json_free_serialized_string(user.serialized_value);
					user.cookie = NULL;
					user.token = NULL;
				}

				if (message != NULL){
					free (message);
				}

				if (response != NULL){
					free (response);
				}
			}

			// Close the connection to the server
			close_connection(sockfd);
		}
		// Check if the user input is "login"
		else if (strcmp(buffer, "login") == 0) {
			// Check if the user is already logged in
			if (user.cookie != NULL) {
				// Print an error message if the user is already logged in
				fprintf(stderr, "Error: You are already logged in.\n");
			} else {
				// Initialize the user
				user = init_user(user);

				char *message;
				char *response;

				// Generate a POST request message with the necessary data and send it to the server
				message = compute_post_request(HOST, LOGIN, TYPE, user.serialized_value, NULL, 0);
				send_to_server(sockfd, message);

				// Receive the response from the server
				response = receive_from_server(sockfd);

				// Check if the response contains "error"
				char *ret = strstr(response, "error");

				// If the login was successful, print a success message
				if (ret == NULL) {
					char *cookie = strstr(response, "Set-Cookie: ");
					strtok(cookie, "\r");
					user.cookie = malloc(strlen(cookie) * sizeof(char) - 11 * sizeof(char));
					memcpy(user.cookie, cookie + 12, strlen(cookie) - 12 * sizeof(char));
					user.cookie[strlen(cookie) - 12] = '\0';
					printf("You have successfully logged in as %s!\n", user.username);
				} else {
					fprintf(stderr, "Error: Login failed. Please try again.\n");

					//Free user in case of error
					free(user.username);
					free(user.password);
					json_free_serialized_string(user.serialized_value);
					user.cookie = NULL;
					user.token = NULL;
				}

				if (message != NULL){
					free (message);
				}

				if (response != NULL){
					free (response);
				}

				// Close the connection to the server
				close_connection(sockfd);
			}
		}
 		// Check if the user input is "enter_library"
		else if (strcmp(buffer, "enter_library") == 0) {
			// Check if the user is logged in
			if (user.cookie != NULL) {
				char *message;
				char *response = NULL;
				char **cookies;
				int cookie_count = 0;
				// Initialize the headers
				cookies = init_headers(user.cookie, NULL, &cookie_count);

				// Check if the headers were successfully initialized
				if (cookies == NULL) {
					fprintf(stderr, "Error: Unable to generate headers.\n");
				} else {
					// Generate a GET request message with the necessary headers and send it to the server
					message = compute_get_or_delete_request("GET", HOST, ACCESS, NULL, cookies, cookie_count);
					send_to_server(sockfd, message);

					// Receive the response from the server
					response = receive_from_server(sockfd);
					char *ret = strstr(response, "error");

					// If the response does not contain "error", extract the token from the response
					if (ret == NULL) {
						char *tok = strstr(response, "token");
						char *tokk = malloc(strlen(tok) - 7);
						strcpy(tokk, tok + 8);
						tokk[strlen(tokk) - 2] = '\0';
						user.token = tokk;
						printf("Access to library granted!\n");
					} else {
						// If the response contains "error", print an error message
						fprintf(stderr, "Error: Library unavailable. Please try again.\n");
					}

					if (message != NULL){
						free (message);
					}

					if (response != NULL){
						free (response);
					}

					for (int i = 0; i < cookie_count; i++) {
						free(cookies[i]);
					}
					free(cookies);
				}
			} else {
				// If the user is not logged in, print an error message
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}

			// Close the connection to the server
			close_connection(sockfd);
		}
		// Check if the user input is "get_books"
		else if (strcmp(buffer, "get_books") == 0) {
			// Check if the user is logged in
			if (user.cookie != NULL) {
				// Check if the user has access token
				if (user.token != NULL) {
					char *message;
					char *response;
					char **cookies;
					int cookie_count = 0;

					// Initialize the headers with the user's cookie and token
					cookies = init_headers(user.cookie, user.token, &cookie_count);

					// Check if the headers were successfully initialized
					if (cookies == NULL) {
						fprintf(stderr, "Error: Unable to generate headers.\n");
					} else {
						// Generate a GET request message with the necessary headers and send it to the server
						message = compute_get_or_delete_request("GET", HOST, BOOKS, NULL, cookies, cookie_count);
						send_to_server(sockfd, message);

						// Receive the response from the server
						response = receive_from_server(sockfd);
						char *ret = strstr(response, "error");

						if (ret == NULL) {
							JSON_Value* root_value = NULL;
							JSON_Array* books_array = NULL;
							char* json_response = extract_json_array(response);

							// Parse the JSON response and extract the books array
							root_value = json_parse_string(json_response);
							books_array = json_value_get_array(root_value);

							// Get the number of books in the array
							size_t book_count = json_array_get_count(books_array);
							if (book_count == 0) {
								// If there are no books in the library, print an error message
								fprintf(stderr, "Error: There are no books in the library yet.\n");
							} else {
								// Iterate through the books array and print each book's title and ID
								for (size_t i = 0; i < book_count; i++) {
									JSON_Object* book = json_array_get_object(books_array, i);
									const char* title = json_object_get_string(book, "title");
									int id = (int)json_object_get_number(book, "id");
									printf("title: %s and id: %d\n", title, id);
								}
							}
							json_value_free(root_value);
						} else {
							// If the response contains "error", print an error message
							fprintf(stderr, "Error: Library unavailable. Please try again.\n");
						}

						if (message != NULL){
							free (message);
						}

						if (response != NULL){
							free (response);
						}
						for (int i = 0; i < cookie_count; i++) {
							free(cookies[i]);
						}

						free(cookies);
					}
				} else {
					// If the user does not have access token, print an error message
					fprintf(stderr, "Error: You do not have access to the library!\n");
				}
			} else {
				// If the user is not logged in, print an error message
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}

			// Close the connection to the server
			close_connection(sockfd);
		}
		// Check if the user input is "get_book"
		else if (strcmp(buffer, "get_book") == 0) {
			// Check if the user is logged in
			if (user.cookie != NULL) {
				// Check if the user has access token
				if (user.token != NULL) {
					char* message;
					char* response;
					char id[LINELEN];
					char** cookies;
					int cookie_count = 0;

					// Prompt the user to enter the book ID
					printf("id=");
					fgets(id, LINELEN, stdin);

					// Remove the newline character from the ID
					if (id[strlen(id) - 1] == '\n') {
						id[strlen(id) - 1] = '\0';
					}

					// Validate if the entered ID is a number
					int valid = 1;
					for (int i = 0; i < strlen(id); i++) {
						if (id[i] < '0' || id[i] > '9') {
							valid = 0;
							break;
						}
					}

					if (valid) {
						// Construct the final URL with the book ID
						size_t final_url_length = strlen(id) + strlen(BOOKS) + 2;
						char* final_url = malloc(final_url_length * sizeof(char));
						if (final_url == NULL) {
							fprintf(stderr, "Error: Failed to allocate memory.\n");
						} else {
							snprintf(final_url, final_url_length, "%s/%s", BOOKS, id);

							// Generate headers to be added to the request
							cookies = init_headers(user.cookie, user.token, &cookie_count);

							// Check if the headers were successfully initialized
							if (cookies == NULL) {
								fprintf(stderr, "Error: Unable to generate headers.\n");
							} else {
								// Generate a GET request message with the necessary headers and send it to the server
								message = compute_get_or_delete_request("GET", HOST, final_url, NULL, cookies, cookie_count);
								send_to_server(sockfd, message);

								// Receive the response from the server
								response = receive_from_server(sockfd);
								char* ret = strstr(response, "error");

								if (ret == NULL) {
									JSON_Value* root_value;
									JSON_Object* book;
									char* json_response = basic_extract_json_response(response);
									if (json_response != NULL) {
										// Parse the JSON response and extract the book object
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
									// If the response contains "error", print an error message
									fprintf(stderr, "Error: No book was found!\n");
								}

								if (message != NULL){
									free (message);
								}

								if (response != NULL){
									free (response);
								}

								for (int i = 0; i < cookie_count; i++) {
									free(cookies[i]);
								}
								free(cookies);
							}
						}
					} else {
						// If the ID is not a number, print an error message
						fprintf(stderr, "Error: ID is not a number. Please try again.\n");
					}
				} else {
					// If the user does not have access token, print an error message
					fprintf(stderr, "Error: You do not have access to the library!\n");
				}
			} else {
				// If the user is not logged in, print an error message
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}

			// Close the connection to the server
			close_connection(sockfd);
		}
		// Check if the user input is "add_book"
		else if (strcmp(buffer, "add_book") == 0) {
			// Check if the user is logged in
			if (user.cookie != NULL) {
				// Check if the user has access token
				if (user.token != NULL) {
					int error = 0;
					// Initialize a book and check for any errors
					struct Book book = init_book(book, &error);
					char *message;
					char *response;
					char **cookies;
					int cookie_count = 0;
					
					if (error == 0) {
						// Generate headers containing the user's cookie and token
						cookies = init_headers(user.cookie, user.token, &cookie_count);
						
						// Check if the headers were successfully initialized
						if (cookies == NULL) {
							fprintf(stderr, "Error: Unable to generate headers.\n");
						} else {
							// Create a POST request to add the book to the library
							message = compute_post_request(HOST, BOOKS, TYPE, book.serialized_value, cookies, cookie_count);
							send_to_server(sockfd, message);
							response = receive_from_server(sockfd);
							
							// Check if any error occurred during the request
							char* ret = strstr(response, "error");
							if (ret != NULL) {
								fprintf(stderr, "Error: %s\n", ret + 7);
							}
							
							printf("You have successfully added %s to the library!\n", book.title);

							if (message != NULL){
								free (message);
							}

							if (response != NULL){
								free (response);
							}

							for (int i = 0; i < cookie_count; i++) {
								free(cookies[i]);
							}
							free(cookies);
						}
					} else {
						// If the information was wrong or incomplete, print an error message
						fprintf(stderr, "Error: The information was wrong or incomplete. Please try again.\n");
					}
					//Free the allocated memory for the book
					free(book.title);
					free(book.author);
					free(book.genre);
					free(book.publisher);
					if (error == 0)
						json_free_serialized_string(book.serialized_value);

				} else {
					// If the user does not have access token, print an error message
					fprintf(stderr, "Error: You do not have access to the library!\n");
				}
			} else {
				// If the user is not logged in, print an error message
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}

			// Close the connection to the server
			close_connection(sockfd);
		}
		// Check if the user input is "delete_book"
		else if (strcmp(buffer, "delete_book") == 0) {
			// Check if the user is logged in
			if (user.cookie != NULL) {
				// Check if the user has access token
				if (user.token != NULL) {
					char* message;
					char* response;
					char** cookies;
					char id[LINELEN];
					int cookie_count = 0;

					// Prompt the user to enter the ID of the book to be deleted
					printf("id=");
					fgets(id, LINELEN, stdin);

					// Remove the trailing newline character from the ID
					if (id[strlen(id) - 1] == '\n') {
						id[strlen(id) - 1] = '\0';
					}

					// Check if the ID contains only digits
					int is_valid_id = 1;
					for (int i = 0; i < strlen(id); i++) {
						if (id[i] < '0' || id[i] > '9') {
							is_valid_id = 0;
							break;
						}
					}

					if (!is_valid_id) {
						fprintf(stderr, "Error: ID must be a number. Please try again.\n");
					} else {
						// Generate headers for the request
						cookies = init_headers(user.cookie, user.token, &cookie_count);

						// Check if the headers were successfully initialized
						if (cookies == NULL) {
							fprintf(stderr, "Error: Unable to generate headers.\n");
						} else {
							// Construct the final URL for the delete request
							char* final_url = malloc(strlen(BOOKS) + strlen(id) + 2);
							if (final_url == NULL) {
								fprintf(stderr, "Error: Failed to allocate memory.\n");
							} else {
								sprintf(final_url, "%s/%s", BOOKS, id);

								// Create the delete request message
								message = compute_get_or_delete_request("DELETE", HOST, final_url, NULL, cookies, cookie_count);
								// Send the request to the server
								send_to_server(sockfd, message);
								// Receive the response from the server
								response = receive_from_server(sockfd);
								// Check if the response contains an error message
								char* ret = strstr(response, "error");

								if (ret != NULL) {
									fprintf(stderr, "Error: No book at specified ID!\n");
								} else {
									printf("Book with ID %s has been successfully deleted from the library!\n", id);
								}

								free(final_url);
							}

							if (message != NULL){
								free (message);
							}

							if (response != NULL){
								free (response);
							}

							for (int i = 0; i < cookie_count; i++) {
								free(cookies[i]);
							}
							free(cookies);
						}
					}

				} else {
					// If the user does not have access token, print an error message
					fprintf(stderr, "Error: You do not have access to the library!\n");
				}
			} else {
				// If the user is not logged in, print an error message
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}

			// Close the connection to the server
			close_connection(sockfd);
		}
		// Check if the user input is "logout"
		else if (strcmp(buffer, "logout") == 0) {
			// Check if the user is logged in
			if (user.cookie != NULL) {
				char* message;
				char* response;
				char** cookies;
				int cookie_count;

				// Generate headers for the request
				cookies = init_headers(user.cookie, user.token, &cookie_count);

				// Create the logout request message
				message = compute_get_or_delete_request("GET", HOST, LOGOUT, NULL, cookies, cookie_count);
				// Send the request to the server
				send_to_server(sockfd, message);
				// Receive the response from the server
				response = receive_from_server(sockfd);
				// Check if the response contains an error message
				char* ret = strstr(response, "error");

				if (ret != NULL) {
					fprintf(stderr, "Error: %s\n", ret + 7);
				}

				// Clear the user's cookie and token
				free(user.username);
				free(user.password);
				if (user.cookie != NULL){
					free (user.cookie);
					user.cookie = NULL;
				}

				if (user.token != NULL){
					free (user.token);
					user.token = NULL;
				}
				json_free_serialized_string(user.serialized_value);
				if (message != NULL){
					free (message);
				}
				if (response != NULL){
					free (response);
				}
				for (int i = 0; i < cookie_count; i++) {
					free(cookies[i]);
				}
				free(cookies);
				printf("Logout successful!\n");

				// Close the connection to the server
				close_connection(sockfd);
			} else {
				// If the user is not logged in, print an error message
				fprintf(stderr, "Error: You are not currently logged in!\n");
			}
		}
		// Check if the user input is "exit"
		else if (strcmp (buffer, "exit") == 0) {
			// Close the connection to the server and free user
			if (user.cookie != NULL){
				free (user.cookie);
				free(user.username);
				free(user.password);
				if (user.token != NULL){
					free (user.token);
				}
				json_free_serialized_string(user.serialized_value);
			}
			close_connection(sockfd);
			break;
		}
		else {
			// If the command is wrong, print an error message
			fprintf (stderr, "Error: Please enter a valid command.\n");
		}
	}
	return 0;
}
