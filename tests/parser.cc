#include "uchuva.h"

int main() {
  const std::string input =
    "GET /root HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "User-Agent: curl/7.68.0\r\n"
    "Accept: */*\r\n"
    "\r\n"
    "Body starts here";
  HttpParser parser(input);
  parser.ParseHTTP();
  return 0;
}
