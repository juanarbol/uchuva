#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

#include "tcpclient.h"


TCPClient::TCPClient(EventLoop& loop, int fd) :
  loop_(loop),
  closed_(false),
  fd_(fd) {
  loop_.addFd(fd, EPOLLIN, [this](uint32_t events) {
    handleEvent(events);
  });
};


TCPClient::~TCPClient() {
  close();
}


void TCPClient::onData(std::function<void(const std::string&)> cb) {
  on_data_ = std::move(cb);
}


void TCPClient::onClose(std::function<void()> cb) {
  on_close_ = std::move(cb);
}


void TCPClient::handleEvent(uint32_t events) {
  if (closed_) return;

  if (events & (EPOLLHUP | EPOLLERR)) {
    close();
    return;
  }

  if (events & EPOLLIN) {
    char buf[4096];
    ssize_t n = ::recv(fd_, &buf, sizeof(buf), 0);
    if (n > 0) {
      std::string data(buf, n);
      if (on_data_) on_data_(data);  // call the callback with the buffer.
    } else {
      close();
      return;
    }
  }

  if (events & EPOLLOUT) {
    if (!write_buffer_.empty()) {
      ssize_t n = ::send(fd_, write_buffer_.data(), write_buffer_.size(), MSG_NOSIGNAL);
      if (n > 0) write_buffer_.erase(0, n);
    } else {
      loop_.updateFd(fd_, EPOLLIN);  // No longer need EPOLLOUT
    }
  }
}


void TCPClient::write(const std::string& data) {
  if (closed_) return;

  ssize_t n = ::send(fd_, data.data(), data.size(), MSG_NOSIGNAL);
  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // buffer
      write_buffer_ += data;
      loop_.updateFd(fd_, EPOLLIN | EPOLLOUT);
    } else {
      fprintf(stderr, "Error in send: %d", errno);
      close();
    }
  } else if ((size_t)n < data.size()) {
    // partial write
    write_buffer_.append(data.begin() + n, data.end());
    loop_.updateFd(fd_, EPOLLIN | EPOLLOUT);
  }
}


void TCPClient::close() {
  // May be called by server teardown.
  if (closed_) return;
  closed_ = true;

  loop_.removeFd(fd_);
  loop_.defer([this]() {
    // Defer the close FD :-) avoid race conditions
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }

    if (on_close_) on_close_();
  });
}
