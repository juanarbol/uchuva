#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "tcpserver.h"
#include "eventloop.h"


TCPServer::TCPServer(EventLoop& loop) : loop_(loop) {}


TCPServer::~TCPServer() {
  loop_.stop();
  ::close(listen_fd_);
}


void TCPServer::bind(const std::string& host, uint16_t port) {
  listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (listen_fd_ < 0) {
    fprintf(stderr, "Failed creating the TCP socket for the server.\n");
    return;
  }

  int opt = 1;
  ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
    fprintf(stderr, "Invalid bind address.\n");
    return;
  }

  if (::bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "Bind failed.\n");
    return;
  }
}


void TCPServer::listen(std::function<void(TCPClient&)> onConnection) {
  if (::listen(listen_fd_, SOMAXCONN) < 0) {
    fprintf(stderr, "Listen failed.\n");
    return;
  }

  on_connection_ = std::move(onConnection);
  loop_.addFd(listen_fd_, EPOLLIN, [this](uint32_t events) {
    handleAccept(events);
  });
}


void TCPServer::handleAccept(uint32_t events) {
  if (events & (EPOLLERR | EPOLLHUP)) {
    fprintf(stderr, "Server socket error.\n");
    return;
  }

  while (true) {
    int clientFd = ::accept4(listen_fd_, nullptr, nullptr,
                             SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (clientFd < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) break;
      fprintf(stderr, "Server accept error.\n");
      break;
    }

    auto client = std::make_unique<TCPClient>(loop_, clientFd);

    // Transfer ownership first
    clients_.push_back(std::move(client));
    auto& stored = *clients_.back();  // safe reference
    // Call the connection callback before storing
    if (on_connection_) on_connection_(stored);
  }
}


void TCPServer::shutdown() {
  closeAllClients();
}


void TCPServer::closeAllClients() {
  if (clients_.empty()) return;

  for (auto& client : clients_) {
    if (client) client->close();
  }

  clients_.clear();
}
