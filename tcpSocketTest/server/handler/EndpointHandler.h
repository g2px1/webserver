//
// Based on Vadim Zhukov's code, 20.11.2023.
//

#ifndef HANDLER_H
#define HANDLER_H
#include <regex>
#include "data/HttpRequest.h"
#include "data/HttpResponse.h"
#include "data/types.h"

namespace unit::server::regex {
    template<class UserData = std::string>
    class EndpointHandler {
    public:
        virtual ~EndpointHandler() = default;

        virtual std::optional<std::function<void (data::HttpRequest &, data::HttpResponse &)>> match(request::type request_type, const UserData&data) = 0;

        virtual void addHandler(request::type request_type, const std::regex&regular_expression, std::function<void (data::HttpRequest &, data::HttpResponse &)> function) = 0;

    private:
        virtual bool equalsFunction(request::type request_type, const UserData&data, const std::regex&e) = 0;
    };
}; // unit::server::regex
#endif //HANDLER_H
