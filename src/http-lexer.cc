#include <cctype>
#include <algorithm>
#include <stdio.h>
#include <string_view>

#include "http-lexer.h"
#include "http-token.h"

namespace {
int isDigit(int n) { return std::isdigit(n); }
//  int isEndline(char ch) { return ch == '\n'; }
int isLetter(char ch) { return std::isalpha(ch); }
bool isSpace(char ch) {
  return ch == ' '  ||
         ch == '\t' ||
         ch == '\n' ||
         ch == '\r';
}
} // helpers

Lexer::Lexer(const std::string& input) :
  input_(input), op_(0), ch_('\0'), position_(0), readPosition_(0) {
  readChar();  // Initial set of position and readPosition
}

/* Getters */
char Lexer::peek() {
  if (readPosition_ >= input_.size())
    return 0;

  return input_[readPosition_];
}

int Lexer::position() { return position_; }

int Lexer::redPosition() { return readPosition_; }

/* internals/private */
void Lexer::readChar() {
  if (readPosition_ >= input_.size()) {
    ch_ = '\0';
  } else {
    ch_ = input_[readPosition_];
  }

  position_ = readPosition_;
  readPosition_ += 1;
}

void Lexer::skipSpace() {
  while (isSpace(ch_))
    readChar();
}

Token Lexer::consumeMethod() {
  skipSpace();
  int pos = position_;
  while (isLetter(ch_))
    readChar();

  std::string_view lex(input_.data() + pos, position_ - pos);

  // Tell the parser to move to the URL now
  op_ = URL;

  // Not found? Not soported
  auto it = std::find(METHOD_SET.begin(), METHOD_SET.end(), lex);
  if (it != METHOD_SET.end())
    return Token{Token::Type::METHOD, lex};
  else
    return Token{Token::Type::INVALID, lex};
}

Token Lexer::consumeUrl() {
  skipSpace();

  int pos = position_;
  readChar(); // read '/'

  while (isLetter(ch_))
    readChar();

  std::string_view lex(input_.data() + pos, position_ - pos);

  // Tell the parser to move to the version now
  op_ = VERSION;

  return Token{Token::Type::URL, lex};
}

Token Lexer::consumeVersion() {
  skipSpace();

  int pos = position_;

  // We need to move the cursor thru' HTTP/X.Y
  while (isLetter(ch_) ||
         isDigit(ch_)  ||
         ch_ == '/'    ||
         ch_ == '.')
    readChar();

  std::string_view lex(input_.data() + pos, position_ - pos);

  // Tell the parser to move to the version now
  op_ = HEADER;

  return Token{Token::Type::VERSION, lex};
}

std::string_view Lexer::readLine() {
  int start = position_;

  // Case: CRLF CRLF (end of headers)
  if ((ch_ == '\r' && peek() == '\n')) {
    // Check next two bytes
    if (readPosition_ + 1 < input_.size() &&
        input_[readPosition_ + 1] == '\r' &&
        readPosition_ + 2 < input_.size() &&
        input_[readPosition_ + 2] == '\n') {
      // Do NOT consume them here — let the caller handle BODY transition
      return std::string_view();  // empty line
    }
  }

  // Read until end of line
  while (ch_ != '\r' && ch_ != '\n' && ch_ != '\0')
    readChar();

  // Handle CRLF / LF
  if (ch_ == '\r' && peek() == '\n') {
    readChar();
    readChar();
  } else if (ch_ == '\r' || ch_ == '\n') {
    readChar();
  }

  return std::string_view(input_.data() + start, position_ - start - 1);
}

Token Lexer::consumeHeader() {
  std::string_view line = readLine();

  // Empty line means end-of-headers
  if (line.empty()) {
    op_ = BODY;
    return NextToken();
  }

  size_t sep = line.find(':');
  if (sep == std::string_view::npos)
    return Token{Token::Type::INVALID, line};

  return Token{Token::Type::HEADER, line};
}

Token Lexer::consumeBody() {
  int pos = position_;

  while (ch_ != '\0')
    readChar();

  std::string_view lex(input_.data() + pos, position_ - pos);

  op_ = EOI;
  return Token{Token::Type::BODY, lex};
}

Token Lexer::NextToken() {
  skipSpace();

  // Handle EOF reached
  if (ch_ == EOF || ch_ == '\0') return Token{Token::Type::EOI, "NIL"};

  switch (op_) {
    case METHOD:
      return consumeMethod();
    case URL:
      return consumeUrl();
    case VERSION:
      return consumeVersion();
    case HEADER:
      return consumeHeader();
    case BODY:
      return consumeBody();
    case EOI:
      return Token{Token::Type::EOI, ""};
    default:
      return Token{.type = Token::Type::INVALID, .literal = "WTF" };
  }
}
