#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "eventloop.h"
#include "tcpclient.h"

class TCPClient;

class TCPServer {
public:
  TCPServer(EventLoop& loop);
  ~TCPServer();

  void bind(const std::string& host, uint16_t port);
  void listen(std::function<void(TCPClient&)> onConnection);
  void closeAllClients();
  void shutdown();

private:
  void handleAccept(uint32_t events);

  EventLoop& loop_;
  int listen_fd_;
  std::function<void(TCPClient&)> on_connection_;
  std::vector<std::unique_ptr<TCPClient>> clients_;
};

#endif // TCP_SERVER_H
