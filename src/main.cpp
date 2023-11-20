#include "handler/Server.h"
#include "request/requestTypesStr.h"

int main() {
    unit::server::handler::Server server("/Users/kirillzhukov/Desktop/config.toml");
    server.handle(unit::server::request::GET, "/*",
                  [](unit::server::data::HttpRequest&request, unit::server::data::HttpResponse&response) {
                      response.writeJSON(boost::json::parse(R"({"test":"new test"})"));
                      response.addHeader((char *) ":status", (char *) "200");
                      response.addHeader((char *) "content-type", (char *) "application/json");
                  });
    server.start();

    return 0;
}
