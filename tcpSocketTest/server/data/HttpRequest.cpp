//
// Created by Kirill Zhukov on 10.11.2023.
//

#include "HttpRequest.h"

#include <boost/log/trivial.hpp>

unit::server::data::HttpRequest::HttpRequest(bool isFinished, const std::vector<unsigned char> &data, int32_t stream_id)
        : data(data), stream_id(stream_id) {
}

unit::server::data::HttpRequest::HttpRequest(int32_t stream_id) : stream_id(stream_id) {}

int32_t unit::server::data::HttpRequest::getStreamId() const {
    return stream_id;
}


const std::vector<unsigned char> &unit::server::data::HttpRequest::getData() {
    return data;
}

void unit::server::data::HttpRequest::parseUrl(const std::string& url) {
    this->url = {url};
}

int32_t unit::server::data::HttpRequest::getStreamId() {
    return this->stream_id;
}
