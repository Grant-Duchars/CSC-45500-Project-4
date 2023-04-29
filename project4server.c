#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Error constants
const int USAGE_ERROR = 1;
const int SOCKET_ERROR = 2;
const int BIND_ERROR = 3;
const int LISTEN_ERROR = 4;
// Number of total concurrent connections allowed
const int MAX_WAITERS = 25;
// Global accumulator
int ACCUMULATOR;
// Struct for passing arguments to the thread for servicing the client
struct DoWorkStruct {
  int conn_sock;
  struct sockaddr_in *client_addr;
};
// Function hoisting
int do_server(int port_num);
void *do_work(void *args);
int check_commands(char *input, int conn_sock);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "ERROR: Usage: %s <port-number>\n", argv[0]);
    exit(USAGE_ERROR);
  }
  ACCUMULATOR = 0;
  return do_server(strtol(argv[1], NULL, 10));
}

int do_server(int port_num) {
  // Set up the socket
  int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (!listen_socket) {
    fprintf(stderr, "ERROR: Could not create listening socket\n");
    exit(SOCKET_ERROR);
  }
  struct sockaddr_in local_addr;
  local_addr.sin_family = AF_INET; // IPv4
  local_addr.sin_addr.s_addr = INADDR_ANY;
  local_addr.sin_port = htons(port_num);
  // Bind to socket
  if (bind(listen_socket, (struct sockaddr *)&local_addr, sizeof(local_addr))) {
    fprintf(stderr, "ERROR: bind() error\n");
    exit(BIND_ERROR);
  }
  // Listen on socket for connections
  if (listen(listen_socket, MAX_WAITERS)) {
    fprintf(stderr, "ERROR: listen() error\n");
    exit(LISTEN_ERROR);
  }
  // Main loop waiting for clients to connect
  while (1) {
    // Accept a new connection from a client
    struct sockaddr_in from_addr;
    unsigned int from_len = sizeof(from_addr);
    int connected_socket =
        accept(listen_socket, (struct sockaddr *)&from_addr, &from_len);
    printf("Recieved new connection\n");
    // Set up parameters for the new thread
    pthread_t *thid = malloc(sizeof(pthread_t));
    struct DoWorkStruct *params = malloc(sizeof(struct DoWorkStruct));
    params->conn_sock = connected_socket;
    params->client_addr = &from_addr;
    // Go off and service client
    pthread_create(thid, NULL, do_work, (void *)params);
    pthread_join(*thid, NULL);
    free(thid);
  }
}

void *do_work(void *args) {
  // Destructure the arguments
  struct DoWorkStruct *struct_args = (struct DoWorkStruct *)args;
  int conn_sock = struct_args->conn_sock;
  struct sockaddr_in *client_addr = struct_args->client_addr;
  free(struct_args);
  // Create a buffer to recieve the command
  char *buffer = malloc(80 * sizeof(char));
  // Read the command from the user
  read(conn_sock, buffer, 80);
  // Check for valid command and service if so
  if (check_commands(buffer, conn_sock)) {
    printf("Invalid command: %s", buffer);
  }
  // Close the connection and free the command buffer
  close(conn_sock);
  free(buffer);
  return NULL;
}

int check_commands(char *input, int conn_sock) {
  // Check client input for specific commands
  if (strstr(input, "get") != NULL) {
    printf("Running get\n");
    // Create string from accumulator to send to client
    char *formatted = malloc(50 * sizeof(char));
    snprintf(formatted, 49, "%d\n", ACCUMULATOR);
    // Send the string
    write(conn_sock, formatted, 50);
    // Free the string
    free(formatted);
  } else if (strstr(input, "add") != NULL) {
    printf("Running add\n");
    // Create string from accumulator to send to client
    char *formatted = malloc(50 * sizeof(char));
    ACCUMULATOR += strtol(strchr(input, ' '), NULL, 10);
    snprintf(formatted, 49, "%d\n", ACCUMULATOR);
    // Send the string
    write(conn_sock, formatted, 50);
    // Free the string
    free(formatted);
  } else if (strstr(input, "clear") != NULL) {
    printf("Running clear\n");
    // Don't need to send anything to client
    ACCUMULATOR = 0;
  } else {
    // Invalid Command
    return 1;
  }
  return 0;
}