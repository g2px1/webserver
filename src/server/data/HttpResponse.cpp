//
// Created by Kirill Zhukov on 15.11.2023.
//

#include "HttpResponse.h"

unit::server::data::HttpResponse::HttpResponse(const int32_t stream_id) : stream_id(stream_id), type(NONE) {}

bool unit::server::data::HttpResponse::writeJSON(const boost::json::value&res) {
    if (this->type != NONE) {
        return false;
    }
    boost::json::serializer sr;
    sr.reset(&res);

    std::vector<uint8_t> temp_buffer;
    while (!sr.done()) {
        char buf[4096];
        auto sv = sr.read(buf, sizeof(buf));
        temp_buffer.insert(temp_buffer.end(), sv.data(), sv.data() + sv.size());
    }

    this->data = std::move(temp_buffer);
    return true;
}

bool unit::server::data::HttpResponse::writeFile(const std::string&file_path) {
    if (this->type != NONE) {
        return false;
    }
    int fd = open(file_path.c_str(), O_RDONLY);

    if (fd == -1) {
        BOOST_LOG_TRIVIAL(info) << "File not opened: " << file_path << ", errno: " << errno;
        return false;
    }
    this->data = fd;
    this->type = FD;
    return true;
    // struct stat file_status;
    // if (stat(file_path.c_str(), &file_status) < 0) {
    //     return;
    // }
    //
    // this->buffer.resize(file_status.st_size);
    //
    // ssize_t r = read(fd, this->buffer.data(), file_status.st_size);
    //
    // if (r == -1) {
    //     BOOST_LOG_TRIVIAL(info) << "File not readed: " << errno;
    // }
    //
    // for (auto&sym: this->buffer) {
    //     std::cout << (char) sym;
    // }
}

bool unit::server::data::HttpResponse::writeRawData(const uint8_t* buf, const size_t length) {
    if (this->type != NONE) {
        return false;
    }
    this->data = std::vector<uint8_t>(buf, buf + length);
    return true;
}

const std::vector<uint8_t>& unit::server::data::HttpResponse::getBuffer() const {
    if (std::holds_alternative<std::vector<uint8_t>>(data)) {
        return std::get<std::vector<uint8_t>>(data);
    } else {
        static std::vector<uint8_t> empty_buffer;
        return empty_buffer; // Return an empty buffer if the variant does not hold a buffer
    }
}
