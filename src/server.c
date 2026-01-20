#include "engine.h"
#include "expires.h"
#include "hashtable.h"
#include "result.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 8080
#define MAX_INPUT 256

void start_server(HashTable *ht) {
  int server_fd, client_fd;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  char input[MAX_INPUT];

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(server_fd, 1) < 0) {
    perror("listen");
    exit(1);
  }

  printf("[server] listening on port %d\n", PORT);

  client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
  if (client_fd < 0) {
    perror("accept");
    exit(1);
  }

  printf("[server] client connected\n");

  FILE *client_fp = fdopen(client_fd, "r+");
  if (!client_fp) {
    perror("fdopen");
    close(client_fd);
    exit(1);
  }

  while (fgets(input, sizeof(input), client_fp)) {
    expire_sweep(ht, 10);

    Result r = execute_command(ht, input);
    print_result(client_fp, &r);
    fflush(client_fp);
    free_result(&r);
  }

  printf("[server] client disconnected\n");

  fclose(client_fp);
  close(server_fd);
}
