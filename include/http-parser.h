#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "http-lexer.h"
#include "http-token.h"
#include <string>
#include <vector>

class HttpParser {
public:
  HttpParser(const std::string& input);

  void ParseHTTP();
private:
  void NextToken();
  void reportInvalidToken();

  std::string input_;
  Lexer lexer_;

  Token curToken_;
  Token peekToken_;
  std::vector<std::string> errors;
};

#endif  // HTTP_PARSER_H
