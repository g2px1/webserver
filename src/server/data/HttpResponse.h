//
// Created by Kirill Zhukov on 15.11.2023.
//

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "boost/json.hpp"
#include "boost/log/trivial.hpp"


namespace unit::server {
    namespace data {
        enum DATA_TYPE {
            NONE = 0,
            BUFFER = 1,
            FD = 2
        };
        class HttpResponse {
            friend class HttpSession;

        public:
            explicit HttpResponse(int32_t stream_id);

            bool writeJSON(const boost::json::value &res);

            bool writeFile(const std::string&file_path);

            bool writeRawData(const uint8_t *buf, size_t length);

            [[nodiscard]] const std::vector<uint8_t>& getBuffer() const;

        private:
            // union {
            //     std::vector<uint8_t> buffer;
            //     int fd;
            // };
            std::variant<std::vector<uint8_t>, int> data;
            size_t read_offset = 0;
            const int32_t stream_id;
            DATA_TYPE type = NONE;
        };
    }; // data
}

; // unit::server

#endif //HTTPRESPONSE_H
