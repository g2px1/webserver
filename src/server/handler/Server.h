//
// Created by Kirill Zhukov on 15.11.2023.
//

#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <functional>
#include "SessionManager.h"
#include "configuration/ConfigReader.h"
#include "data/HttpRequest.h"

namespace unit::server {
    namespace handler {
        class Server {
        public:
            explicit Server(const std::string&config_path);

            void handle(request::type request_type, const std::string &endpoint, std::function<void (configuration::ConfigReader &, data::HttpRequest &, data::HttpResponse &)> function);

            [[noreturn]] void start();

        private:
            configuration::ConfigReader config;
            std::vector<std::thread> iocs;
            std::unordered_map<requestHandlerKey::RequestKey, std::function<void (configuration::ConfigReader &, data::HttpRequest &, data::HttpResponse &)>,
                requestHandlerKey::RequestKeyHash, requestHandlerKey::RequestKeyEqual> handlers;
        };
    }; // handler
}; // unit::server

#endif //SERVER_H
