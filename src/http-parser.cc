#include "http-parser.h"
#include "http-lexer.h"
#include <stdio.h>

HttpParser::HttpParser(const std::string& input) : input_(input), lexer_(input) {
  // Set peekToken and currToken
  NextToken();
  NextToken();
}

void HttpParser::NextToken() {
  // Move on on the buffer
  curToken_ = peekToken_;
  peekToken_ = lexer_.NextToken();
}

void HttpParser::ParseHTTP() {
  while (curToken_.type != Token::Type::EOI) {
    fprintf(stderr, "PARSED LITERAL: %.*s\n",
            (int)curToken_.literal.size(),
            curToken_.literal.data());
    NextToken();
  }

  fprintf(stdout, "We're done!\n");
}
