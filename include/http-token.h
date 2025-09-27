#ifndef HTTP_TOKEN_H
#define HTTP_TOKEN_H

#include <string_view>
#include <unordered_set>

/* This is my lookup table for reserved words? */
static const std::unordered_set<std::string_view> METHOD_SET = {
  "OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"
};


struct Token {
  enum class Type { METHOD, URL, VERSION, HEADER, BODY, INVALID, EOI };

  Type type;
  std::string_view literal;
};

#endif  // HTTP_TOKEN_H
