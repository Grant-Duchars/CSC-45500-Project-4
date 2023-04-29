#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int SUCCESS = 0;
const int CLI_ARGS_ERROR = 1;
const int GET_HOST_ERROR = 2;
const int SOCKET_ERROR = 3;
const int CONNECT_ERROR = 4;
const int READ_ERROR = 5;

int do_client(char *server_name, char *server_port, char *command);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "ERROR: Usage: %s <host-name> <host-port>\n", argv[0]);
    return CLI_ARGS_ERROR;
  }
  // Prompt user to enter a command to send to server
  printf(">>> ");
  fflush(stdout);
  char *buffer = malloc(80 * sizeof(char));
  fgets(buffer, 80, stdin);
  // Connect to server and send the command
  int res = do_client(argv[1], argv[2], buffer);
  free(buffer);
  exit(res);
}

int do_client(char *server_name, char *server_port, char *command) {
  // Convert server name to ipv4 address
  struct hostent *server_entry;
  server_entry = gethostbyname(server_name);
  if (!server_entry) {
    fprintf(stderr, "ERROR: gethostbyname() failed on: %s\n", server_name);
    return GET_HOST_ERROR;
  }
  // Setup connection info for socket
  struct sockaddr_in server_info;
  server_info.sin_family = AF_INET; // Using IPv4
  server_info.sin_addr = *(struct in_addr *)server_entry->h_addr_list[0];
  server_info.sin_port = htons(strtol(server_port, NULL, 10));
  int my_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (!my_socket) {
    fprintf(stderr, "ERROR: socket() failed\n");
    return SOCKET_ERROR;
  }
  // Connect to server with the socket
  if (connect(my_socket, (struct sockaddr *)&server_info,
              sizeof(server_info))) {
    fprintf(stderr, "ERROR: connect() failed\n");
    return CONNECT_ERROR;
  }
  // Send command to server
  write(my_socket, command, 80);
  char *buffer = malloc(80 * sizeof(char));
  // Get the response from the server
  int chars_read = read(my_socket, buffer, 80);
  if (chars_read > 0) {
    printf("%s", buffer);
  }
  // Close the server connection and free the recieve buffer
  close(my_socket);
  free(buffer);
  return SUCCESS;
}