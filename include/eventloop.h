#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <cstdint>
#include <unordered_map>
#include <functional>

/* This a TCPServer event loop */
class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  void run();
  void stop();

  void addFd(int fd, uint32_t events, std::function<void(uint32_t)> cb);
  void updateFd(int fd, uint32_t events);
  void removeFd(int fd);

  void defer(std::function<void()> cb);

private:
  int backend_fd_;  // epoll or kqueue
  bool running_;


  // Map int (fd) and callbacks for each fd
  std::unordered_map<int, std::function<void(uint32_t)>> callbacks_;
  // Defer
  std::vector<std::function<void()>> defered_;
};

#endif  // EVENT_LOOP_H
