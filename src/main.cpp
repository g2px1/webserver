#include "handler/Server.h"

int main() {
    unit::server::handler::Server server("/Users/kirillzhukov/Desktop/config.toml");
    server.handle(unit::server::request::POST, "/",
                  [](unit::server::configuration::ConfigReader &config, unit::server::data::HttpRequest &request,
                     unit::server::data::HttpResponse &response) {

                      response.writeJSON(boost::json::parse(R"({"test":"test"})"));

                  });
    server.start();
}
