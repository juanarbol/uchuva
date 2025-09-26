#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <cstdint>
#include <functional>
#include <string>

#include "eventloop.h"

class TCPClient {
public:
  TCPClient(EventLoop& loop, int fd);
  ~TCPClient();

  void onData(std::function<void(const std::string&)> cb);
  void onClose(std::function<void()> cb);
  void handleEvent(uint32_t events);

  void write(const std::string&);
  void close();

private:
  EventLoop& loop_;
  bool closed_;
  int fd_;
  std::function<void()> on_close_;
  std::function<void(const std::string&)> on_data_;
  std::string write_buffer_;
};

#endif  // TCPCLIENT_H
