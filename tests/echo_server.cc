#include <stdio.h>

#include "uchuva.h"

int main() {
  EventLoop loop;

  TCPServer server(loop);
  server.bind("0.0.0.0", 3000);

  server.listen([&](TCPClient& client) {
    fprintf(stdout, "New client connected! \n");
    client.onData([&](const std::string& data) {
      if (data == "FIN\n" || data == "FIN\r\n" || data == "FIN") {
        client.write("Goodbye!");
        client.close();
        loop.stop();
        return;
      }

      const std::string res = "HTTP/1.1 200 OK\r\n"
                              "Content-Length: 12\r\n"
                              "Content-Type: text/plain\r\n"
                              "\r\n"
                              "Hello world!";
      client.write(res.data()); // echo back
    });

    client.onClose([] {
      fprintf(stdout, "Client disconnected\n");
    });
  });

  loop.run();

  return 0;
}
