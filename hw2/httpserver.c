#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>

#include "libhttp.h"

/*
 * Global configuration variables.
 * You need to use these in your implementation of handle_files_request and
 * handle_proxy_request. Their values are set up in main() using the
 * command line arguments (already implemented for you).
 */
int server_port;
char *server_files_directory;
char *server_proxy_hostname;
int server_proxy_port;

char *getStringFromFile (FILE *file) {
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *string = malloc(fsize + 1);

    fread(string, fsize, 1, file);


    FILE *wp;
    wp = fopen("test_file.txt", "wb");
    fwrite(string, fsize, 1, wp);
    fclose(file);
    fclose(wp);
    // string[fsize] = '\0';

    // char *retString = malloc(fsize + 50);
    // char front[] = "<html><body><a href=’/’>";
    // char back[] = "</a></body></html>";

    // strcpy(retString, front);
    // strcat(retString, string);
    // strcat(retString, back);
    return string;
}

/*
 * Reads an HTTP request from stream (fd), and writes an HTTP response
 * containing:
 *
 *   1) If user requested an existing file, respond with the file
 *   2) If user requested a directory and index.html exists in the directory,
 *      send the index.html file.
 *   3) If user requested a directory and index.html doesn't exist, send a list
 *      of files in the directory with links to each.
 *   4) Send a 404 Not Found response.
 */
void handle_files_request(int fd) {

  /* YOUR CODE HERE (Feel free to delete/modify the existing code below) */
  int isDir;
  struct http_request *request = http_request_parse(fd);

  char *files = server_files_directory;
  char *path = request->path;

  if (*(path + strlen(path) - 1) == '/') {
    isDir = 1;
  } else {
    isDir = 0;
  }

  printf("method: %s\n", request->method);
  printf("path: %s\n", path);
  printf("dir: %d\n", isDir);

  char *path_requested = strcat(files, ++path);
  strcat(path_requested, "\0");
  printf("path requested: %s\n", path_requested);

struct stat s;
if( stat(path_requested, &s) == 0 )
{
    if( s.st_mode & S_IFDIR ) /* path_requested is a directory. */
    {
      char index[] = "index.html";
      char indexFilename[200];
      strcpy(indexFilename, path_requested);
      strcat(indexFilename, index);
      FILE *indexFile = fopen(indexFilename, "rb");

      if (indexFile == NULL) { /* No index file in directory requested. */

          DIR *dir = opendir(path_requested);
          struct dirent *dp;
          char retString[500];
          while ((dp = readdir (dir)) != NULL) {
            // printf("Directory: %s\n", dp->d_name);
            // if (dp->d_type != DT_DIR && dp->d_type != DT_REG) continue;
            if (strcmp(dp->d_name, ".") == 0) continue;
            if (strcmp(dp->d_name, "..") == 0) continue;
            // char test[3];
            // strcpy(test, 3, dp->d_name);
            // if (strcmp(test, "._.") == 0) continue;
            char str[200];
            snprintf(str, sizeof str, "<p><A href=\"./%s\">%s</A></p>\n", dp->d_name, dp->d_name);
            // printf("Directory: %s\n", str);
            strcat(retString, str);
          }
            printf("retString: \n%s\n", retString);

          char pStr[200];
          strcpy(pStr, path_requested);
          char* last_backslash = strrchr(pStr, '/'); 
          printf("last_backslash + 1 = %s\n", last_backslash + 1);
          if (last_backslash++ == '\0') {
            printf("yep\n");
            *last_backslash = '\0';
            last_backslash = strrchr(pStr, '/');
          }
          *last_backslash = '\0';
          printf("parent Dir: %s\n", pStr);
          char parentDir[200];
          snprintf(parentDir, sizeof(parentDir), "<a href=\"../\">%s</a>", pStr);

          http_start_response(fd, 200);
          http_send_header(fd, "Content-type", "text/html");
          http_end_headers(fd);
          http_send_string(fd, retString);
              // <a href="../">"Parent directory"</a>
              // "<p>okayokayokay</p>");
      } else { /* Index file in directory requested. */

          http_start_response(fd, 200);
          http_send_header(fd, "Content-type", "text/html");
          http_end_headers(fd);
          http_send_string(fd, getStringFromFile(indexFile));
      }
      fclose(indexFile);    }
    else if( s.st_mode & S_IFREG ) /* path_requested is a file. */
    {
      // printf("It's a file.\n");

      FILE *requestedFile = fopen(path_requested, "rb");
      fseek(requestedFile, 0, SEEK_END);
      size_t fsize = ftell(requestedFile);
      fseek(requestedFile, 0, SEEK_SET);

      char *returnString = getStringFromFile(requestedFile);

      printf("size: %ld\n", fsize);
      char len[20];
      sprintf(len, "%zu", fsize);
      http_start_response(fd, 200);
      // char *mime = http_get_mime_type(path_requested);
      // strcat(mime, "\r\n");
      // printf("Type: %s\n", mime);
      http_send_header(fd, "Content-type", http_get_mime_type(path_requested));
      // http_get_mime_type(path_requested)
      http_send_header(fd, "Content-length", len);
      http_end_headers(fd);
      // http_send_string( fd, 
      //   "<html>"
      //   "<body>"
      //   );
      http_send_data(fd, returnString, fsize);
          // "<center>"
          // "<h1>Welcome to httpserver!</h1>"
          // "<hr>"
          // "<p>Nothing's here yet.</p>"
          // "</center>");        
      // http_send_string( fd, "</p>");
      // http_send_string( fd, 
      //   "</html>"
      //   "</body>"
      //   );

      // http_send_string(fd,
      //     "<center>"
      //     "<h1>Welcome to httpserver!</h1>"
      //     "<hr>"
      //     "<p>Nothing's here yet.</p>"
      //     "</center>");
      // http_send_data(int fd, char *data, size_t size)
    }
    else
    {
        //something else
    }
}
else
{
    //error
printf("Error here.\n");
http_start_response(fd, 404);
http_send_header(fd, "Content-type", "text/html\r\n");
http_send_string(fd, "Error: 404 Not Found");
}

}

/*
 * Opens a connection to the proxy target (hostname=server_proxy_hostname and
 * port=server_proxy_port) and relays traffic to/from the stream fd and the
 * proxy target. HTTP requests from the client (fd) should be sent to the
 * proxy target, and HTTP responses from the proxy target should be sent to
 * the client (fd).
 *
 *   +--------+     +------------+     +--------------+
 *   | client | <-> | httpserver | <-> | proxy target |
 *   +--------+     +------------+     +--------------+
 */
void handle_proxy_request(int fd) {

  /* YOUR CODE HERE */

}

/*
 * Opens a TCP stream socket on all interfaces with port number PORTNO. Saves
 * the fd number of the server socket in *socket_number. For each accepted
 * connection, calls request_handler with the accepted fd number.
 */
void serve_forever(int *socket_number, void (*request_handler)(int)) {

  struct sockaddr_in server_address, client_address;
  size_t client_address_length = sizeof(client_address);
  int client_socket_number;
  pid_t pid;

  *socket_number = socket(PF_INET, SOCK_STREAM, 0);
  if (*socket_number == -1) {
    perror("Failed to create a new socket");
    exit(errno);
  }

  int socket_option = 1;
  if (setsockopt(*socket_number, SOL_SOCKET, SO_REUSEADDR, &socket_option,
        sizeof(socket_option)) == -1) {
    perror("Failed to set socket options");
    exit(errno);
  }

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(server_port);

  if (bind(*socket_number, (struct sockaddr *) &server_address,
        sizeof(server_address)) == -1) {
    perror("Failed to bind on socket");
    exit(errno);
  }

  if (listen(*socket_number, 1024) == -1) {
    perror("Failed to listen on socket");
    exit(errno);
  }

  printf("Listening on port %d...\n", server_port);

  while (1) {

    client_socket_number = accept(*socket_number,
        (struct sockaddr *) &client_address,
        (socklen_t *) &client_address_length);
    if (client_socket_number < 0) {
      perror("Error accepting socket");
      continue;
    }

    printf("Accepted connection from %s on port %d\n",
        inet_ntoa(client_address.sin_addr),
        client_address.sin_port);

    pid = fork();
    if (pid > 0) {
      close(client_socket_number);
    } else if (pid == 0) {
      // Un-register signal handler (only parent should have it)
      signal(SIGINT, SIG_DFL);
      close(*socket_number);
      request_handler(client_socket_number);
      close(client_socket_number);
      exit(EXIT_SUCCESS);
    } else {
      perror("Failed to fork child");
      exit(errno);
    }
  }

  close(*socket_number);

}

int server_fd;
void signal_callback_handler(int signum) {
  printf("Caught signal %d: %s\n", signum, strsignal(signum));
  printf("Closing socket %d\n", server_fd);
  if (close(server_fd) < 0) perror("Failed to close server_fd (ignoring)\n");
  exit(0);
}

char *USAGE =
  "Usage: ./httpserver --files www_directory/ --port 8000\n"
  "       ./httpserver --proxy inst.eecs.berkeley.edu:80 --port 8000\n";

void exit_with_usage() {
  fprintf(stderr, "%s", USAGE);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_callback_handler);

  /* Default settings */
  server_port = 8000;
  void (*request_handler)(int) = NULL;

  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp("--files", argv[i]) == 0) {
      request_handler = handle_files_request;
      free(server_files_directory);
      server_files_directory = argv[++i];
      if (!server_files_directory) {
        fprintf(stderr, "Expected argument after --files\n");
        exit_with_usage();
      }
    } else if (strcmp("--proxy", argv[i]) == 0) {
      request_handler = handle_proxy_request;

      char *proxy_target = argv[++i];
      if (!proxy_target) {
        fprintf(stderr, "Expected argument after --proxy\n");
        exit_with_usage();
      }

      char *colon_pointer = strchr(proxy_target, ':');
      if (colon_pointer != NULL) {
        *colon_pointer = '\0';
        server_proxy_hostname = proxy_target;
        server_proxy_port = atoi(colon_pointer + 1);
      } else {
        server_proxy_hostname = proxy_target;
        server_proxy_port = 80;
      }
    } else if (strcmp("--port", argv[i]) == 0) {
      char *server_port_string = argv[++i];
      if (!server_port_string) {
        fprintf(stderr, "Expected argument after --port\n");
        exit_with_usage();
      }
      server_port = atoi(server_port_string);
    } else if (strcmp("--help", argv[i]) == 0) {
      exit_with_usage();
    } else {
      fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
      exit_with_usage();
    }
  }

  if (server_files_directory == NULL && server_proxy_hostname == NULL) {
    fprintf(stderr, "Please specify either \"--files [DIRECTORY]\" or \n"
                    "                      \"--proxy [HOSTNAME:PORT]\"\n");
    exit_with_usage();
  }

  serve_forever(&server_fd, request_handler);

  return EXIT_SUCCESS;
}
