#include <stdio.h>

#include "http-parser.h"
#include "uchuva.h"

int main() {
  const std::string input = "GET /root HTTP/1.1\n";
  HttpParser parser(input);
  parser.ParseHTTP();
  return 0;
}
