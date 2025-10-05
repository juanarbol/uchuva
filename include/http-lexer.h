#ifndef HTTP_LEXER_H
#define HTTP_LEXER_H

#include "http-token.h"
#include <string>

enum ops { METHOD, URL, VERSION, HEADER, BODY, EOI };

class Lexer {
public:
  Lexer(const std::string& input);
  ~Lexer() = default;

  /* getters */
  char peek();
  int position();
  int redPosition();

  /* Regular lexer stuff */
  Token NextToken();
private:
  /* internal state */
  void readChar();
  void skipSpace();
  Token consumeMethod();
  Token consumeUrl();
  Token consumeVersion();
  Token consumeHeader();
  Token consumeBody();
  std::string_view readLine();

  const std::string input_;

  int op_;  // Keep track of the phases (method, url, version, headers, body)
  char ch_;
  size_t position_;
  size_t readPosition_;
};

#endif
