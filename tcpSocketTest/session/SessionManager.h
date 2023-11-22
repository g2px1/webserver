//
// Created by Kirill Zhukov on 31.10.2023.
//

#ifndef TCPSOCKETTEST_SESSIONMANAGER_H
#define TCPSOCKETTEST_SESSIONMANAGER_H

#include <set>
#include <memory>
#include <utility>
#include "HttpSession.h"
#include "handler/BasicEndpointHandler.h"
#include "request/requestTypesStr.h"

namespace unit::server {
    namespace manager {
        class SessionManager {
        public:
            SessionManager(std::string key_file, std::string cert_file, const std::shared_ptr<regex::basic::BasicEndpointHandler> &endpointHandler);

            void start_session(boost::asio::ip::tcp::socket socket);

            void remove_session(const std::shared_ptr<HttpSession>&session);

        private:
            std::shared_ptr<request::RequestTypeStr> req_str;
            const std::shared_ptr<regex::basic::BasicEndpointHandler> endpointHandler;
            std::set<std::shared_ptr<HttpSession>> sessions_{};
            const std::string key_file_path;
            const std::string cert_file_path;
        };
    }; // manager
}; // unit::server

#endif //TCPSOCKETTEST_SESSIONMANAGER_H
