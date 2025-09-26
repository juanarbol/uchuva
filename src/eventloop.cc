#include <cerrno>
#include <cstdint>
#include <functional>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "eventloop.h"

EventLoop::EventLoop() : running_(false) {
  backend_fd_ = epoll_create1(0);
  if (backend_fd_ < 0) {
    fprintf(stderr, "Error creating the epoll/kqueue fd");
    exit(1);
  }
}

EventLoop::~EventLoop() {
  close(backend_fd_);
}


void EventLoop::addFd(int fd, uint32_t events, std::function<void(uint32_t)> cb) {
  epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;

  if (epoll_ctl(backend_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
    fprintf(stderr, "Error adding the fd to epoll/kqueue");
    return;
  }

  callbacks_[fd] = std::move(cb);
}


void EventLoop::updateFd(int fd, uint32_t events) {
  epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;

  if (epoll_ctl(backend_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
    fprintf(stderr, "Error updating the fd to epoll/kqueue");
  }
}


void EventLoop::removeFd(int fd) {
  epoll_ctl(backend_fd_, EPOLL_CTL_DEL, fd, nullptr);
  callbacks_.erase(fd);
}


void EventLoop::defer(std::function<void()> cb) {
  defered_.push_back(cb);
}


void EventLoop::run() {
  running_ = true;
  const int MAX_EVENTS = 64;
  epoll_event events[MAX_EVENTS];

  while(running_) {
    int n = epoll_wait(backend_fd_, events, MAX_EVENTS, -1);
    if (n < 0)
      if (errno == EINTR) continue; // interrupt, try again.

    for (int i = 0; i < n; i++) {
      int fd = events[i].data.fd;
      uint32_t ev = events[i].events;

      auto it = callbacks_.find(fd);
      if (it != callbacks_.end()) {
        auto cb = it->second;
        cb(ev);
      }
    }

    if (!defered_.empty()) {
      auto tasks = std::move(defered_);
      for (auto& task : tasks) task();
    }
  }
}


void EventLoop::stop() {
  running_ = false;
  callbacks_.clear();
}
