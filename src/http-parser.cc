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
  for (;;) {
    fprintf(stdout, "Current token: Type=%d, Literal='%.*s'\n",
            static_cast<int>(curToken_.type),
            (int)curToken_.literal.size(),
            curToken_.literal.data());

    if (curToken_.type == Token::Type::EOI)
      break;

    NextToken();
  }

  fprintf(stdout, "We're done!\n");
}
