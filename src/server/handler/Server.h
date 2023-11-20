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
#include "BasicEndpointHandler.h"

namespace unit::server {
    namespace handler {
        class Server {
        public:
            explicit Server(const std::string&config_path);

            void handle(request::type request_type, const std::string&endpoint,
                        const std::function<void (data::HttpRequest&, data::HttpResponse&)>& function);

            [[noreturn]] void start();

        public:
            configuration::ConfigReader config;

        private:
            std::vector<std::thread> iocs;
            regex::basic::BasicEndpointHandler endpoint_handler;
        };
    }; // handler
}; // unit::server

#endif //SERVER_H
