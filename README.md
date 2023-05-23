**Name: Theodor-Ioan Rolea**

**Group: 323CA**

# HW3 PCOM

### Description
* This project focuses on creating a client-server application that uses a REST API.
Helpers, request, and buffer are from the Lab 9 repository, with some small changes
to work with the current requirements. The client has been redesigned from the ground up.
Parson is used to compute JSON objects.

***

### How the program works

* The program awaits a user's command in an infinite loop. There are a total of
9 commands that the server works with: register, login, enter_library, get_books,
add_book, get_book, delete_book, logout, and exit. I am going to explain how
each of them works:

***

#### register
* We first check whether the user is already logged in. In this case, displaying
an error. Otherwise, we initialize a new user, checking if the username and
password are correct (they shouldn't contain any spaces). Then, we send a POST
request to the server with the serialized field values from the user. If we haven't
received any errors, the user has been successfully registered. Else, we free
the allocated memory and display an error.

***

#### login
* We first check whether the user is already logged in. In this case, displaying
an error. Otherwise, we initialize a new user, checking if the username and
password are correct (they shouldn't contain any spaces). Then, we send a POST
request to the server with the serialized field values from the user. If we haven't
received any errors, the user has been successfully logged in, and we can extract the cookie from the server response. Else, we free
the allocated memory and display an error.

***

#### enter_library
* We first check whether the user is logged in. If they're not, we display an error. Otherwise, we initialize the headers and then we send a GET
request to the server. If we haven't received any errors, the user has successfully accessed the library, and we can extract the token from the server response. Else, we free the allocated memory and display an error.

***

#### get_books
* We first check whether the user is logged in and has access to the library. If they're not logged in or they don't have access to the library, we display an error. Otherwise, we initialize the headers and then we send a GET
request to the server. If we haven't received any errors, we extract the book array from the response and print the title and ID for each book. Else, we free the allocated memory and display an error.

***

#### get_book
* We first check whether the user is logged in and has access to the library. If they're not logged in or they don't have access to the library, we display an error. Otherwise, we prompt the user to enter the book ID. If the ID is valid, we initialize the headers and then we send a GET request to the server. If we haven't received any errors, we extract the book fields from the response and print them. Else, we free the allocated memory and display an error.

***

#### add_book
* We first check whether the user is logged in and has access to the library. If they're not logged in or they don't have access to the library, we display an error. Otherwise, we initialize the book we want to add, making sure to check
for empty or invalid fields. Then we initialize the headers and send a POST request to the server. If we haven't received any errors, the book was successfully added to the library. Else, we free the allocated memory and display an error.

***

#### delete_book
* We first check whether the user is logged in and has access to the library. If they're not logged in or they don't have access to the library, we display an error. Otherwise, we prompt the user to enter the book ID. If the ID is valid, we initialize the headers and then we send a DELETE request to the server. If we haven't received any errors, the book was successfully deleted from the library. Else, we free the allocated memory and display an error.

***

#### logout
* We first check whether the user is logged in. If they're not, we display an error. Otherwise, we initialize the headers and then we send a GET request to the server. If we haven't received any errors, the user was successfully logged out, and we free the allocated memory.

***

#### exit
* We first check whether the user is logged in. If they're not, we simply close the connection and break the while loop. Otherwise, we free the allocated memory and then close the connection.

***

### About parson

* For this assignment, I have opted to utilize the Parson library. The decision to choose this library stems from its user-friendly nature and the recommendation by the assignment team. When working with the library, we have access to two fundamental data structures, namely JSON_Object and JSON_Value. To manipulate a JSON string effectively, we require a value that serves as the root of the string. From there, we can extract or add objects to the JSON structure. The JSON_Object provides methods to conveniently insert strings or numbers into a JSON using specific keys. For instance, utilizing the function json_object_set_string(object, "username", "test") would incorporate a field named "username" with the corresponding value "test" into the JSON. To handle JSON arrays, we can utilize JSON_Array to extract individual objects within it, enabling further processing based on our requirements.

***

### Mentions and challenges
* I thoroughly enjoyed working on this assignment. The Lab9 skel was very useful, having most of the functions already implemented. I mainly had to focus on the program logic and error handling.