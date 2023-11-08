//
// Created by Kirill Zhukov on 31.10.2023.
//

#ifndef TCPSOCKETTEST_HTTPSESSION_H
#define TCPSOCKETTEST_HTTPSESSION_H

#include <iostream>
#include <map>
#include "boost/asio.hpp"
#include "openssl/ssl.h"
#include "boost/asio/ssl.hpp"
#include "boost/log/trivial.hpp"
#include "nghttp2/nghttp2.h"
#include "ssl/CTX_util.h"
#include "HttpSessionException.h"

#ifdef __linux__
#include <sstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#endif

#define OUTPUT_WOULDBLOCK_THRESHOLD (1 << 16)

#define MAKE_NV(NAME, VALUE)                                                   \
  {                                                                            \
    (uint8_t *)NAME, (uint8_t *)VALUE, sizeof(NAME) - 1, sizeof(VALUE) - 1,    \
        NGHTTP2_NV_FLAG_NONE                                                   \
  }

namespace unit::server {
    namespace data {
        typedef struct request {
            request(bool isFinished, const std::vector<unsigned char> &data, int32_t stream_id);

            request(int32_t stream_id);

            bool isFinished;
            std::vector<unsigned char> data;
            std::unordered_map<std::string, std::string> headers{};
            const int32_t stream_id;
        private:
        } userData;

        typedef struct response {
            response(const std::string &jsonResponse, size_t readOffset);

            std::string json_response;
            size_t read_offset = 0;

            void setStreamId(int32_t streamId);

        private:
            int32_t stream_id;
        } response;

    }

    namespace callbacks {
        static int on_header_callback(nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name,
                                      size_t namelen, const uint8_t *value, size_t valuelen, uint8_t flags,
                                      void *user_data);

        static int on_frame_recv_callback(nghttp2_session *session, const nghttp2_frame *frame, void *user_data);

        static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                            uint32_t error_code, void *user_data);

        static ssize_t send_callback(nghttp2_session *session, const uint8_t *data,
                                     size_t length, int flags, void *user_data);

        static int on_data_chunk_recv_callback(nghttp2_session *session, uint8_t flags,
                                               int32_t stream_id, const uint8_t *data,
                                               size_t len, void *user_data);


        ssize_t data_provider_callback(nghttp2_session *session, int32_t stream_id,
                                       uint8_t *buf, size_t length, uint32_t *data_flags,
                                       nghttp2_data_source *source, void *user_data);

        void send_response(nghttp2_session *session, int32_t stream_id, unit::server::data::response *data);

        static ssize_t send_callback(nghttp2_session *session, const uint8_t *data,
                                     size_t length, int flags, void *user_data);

        void write_handler(const boost::system::error_code& error, std::size_t bytes_transferred);
    }
}

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    explicit HttpSession(boost::asio::ip::tcp::socket socket, const std::string &key_file,
                         const std::string &cert_file);

    virtual ~HttpSession();

    void start();

    void push_stream_data(int32_t stream_id, unsigned char *data);

    void setDataFinishedByStream(int32_t stream_id, bool value);

    void eraseData(int32_t stream);

    void setHeaders(int32_t stream_id, const std::string &header, const std::string &header_value);

    void pushEmptyUserData(int32_t stream_id);

    unit::server::data::request getUserDataByStream(int32_t stream_id);

    std::map<int32_t, unit::server::data::request>::const_iterator begin() const;

    std::map<int32_t, unit::server::data::request>::const_iterator end() const;

private:

    void do_handshake();

    void do_read();

public:
    boost::asio::ip::tcp::socket socket_;
    std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ssl_socket;
private:
    nghttp2_session *session;
    std::map<int32_t, unit::server::data::request> full_data;
public:
    std::string key_file_path;
    std::string cert_file_path;
};


#endif //TCPSOCKETTEST_HTTPSESSION_H
