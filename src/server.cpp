#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

std::vector<std::string> split(std::string in, std::string splitstr) {
  std::vector<std::string> out;
  size_t pos = 0;
  while ((pos = in.find(splitstr)) != std::string::npos) {
    out.push_back(in.substr(0, pos));
    in.erase(0, pos + splitstr.length());
  }
  out.push_back(in);
  return out;
}

std::string get_url(std::string msg) {
  std::vector<std::string> tokens = split(msg, " ");
  if (tokens.size() < 2) {
    return "";
  }
  return tokens[1];
}

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cout << "Logs from your program will appear here!\n";

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  int client_sock = accept(server_fd, (struct sockaddr *)&client_addr,
                           (socklen_t *)&client_addr_len);

  char msg[65536] = {};
  if (recvfrom(client_sock, msg, sizeof(msg) - 1, 0,
               (struct sockaddr *)&client_addr,
               (socklen_t *)&client_addr_len) == SO_ERROR) {
    std::cerr << "listen failed\n";
    return 1;
  }

  std::vector<std::string> tokens = split(msg, "\r\n");
  std::string url = get_url(msg);
  if (url == "") {
    std::cerr << "Failed to parse URL\n";
    close(server_fd);
    return 1;
  }

  if (url == "/") {
    std::string response = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_sock, response.c_str(), response.length(), 0);
    close(server_fd);
    return 0;
  }

  std::string response = std::string("HTTP/1.1 404 Not Found\r\n\r\n");
  send(client_sock, response.c_str(), response.length(), 0);

  close(server_fd);

  return 0;
}
