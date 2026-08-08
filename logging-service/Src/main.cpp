#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static constexpr const char *SOCKET_PATH = "/var/run/telemetry.sock";
static constexpr const char *LOG_PATH = "/var/log/telemetry.csv";

int main() {
  // Create Unix socket
  int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return 1;
  }

  // Remove stale socket file if it exists
  unlink(SOCKET_PATH);

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

  if (bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  if (listen(server_fd, 1) < 0) {
    perror("listen");
    return 1;
  }

  // Open log file
  FILE *log = fopen(LOG_PATH, "a");
  if (!log) {
    perror("fopen");
    return 1;
  }

  fprintf(stdout, "Logger listening on %s\n", SOCKET_PATH);

  while (true) {
    // Wait for daemon to connect
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0)
      continue;

    fprintf(stdout, "Daemon connected\n");

    char buf[256];
    ssize_t n;
    while ((n = read(client_fd, buf, sizeof(buf) - 1)) > 0) {
      buf[n] = '\0';
      // Write to log file
      fputs(buf, log);
      fflush(log);
      // Also print to stdout
      fputs(buf, stdout);
    }

    close(client_fd);
    fprintf(stdout, "Daemon disconnected\n");
  }
}
