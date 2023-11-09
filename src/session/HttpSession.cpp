//
// Created by Kirill Zhukov on 31.10.2023.
//

#include "HttpSession.h"

unit::server::data::request::request(bool isFinished, const std::vector<unsigned char> &data, int32_t stream_id)
        : isFinished(
        isFinished), data(data), stream_id(stream_id) {}

unit::server::data::request::request(int32_t stream_id) : isFinished(false), stream_id(stream_id) {}

int
unit::server::callbacks::on_header_callback(nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name,
                                            size_t namelen, const uint8_t *value, size_t valuelen, uint8_t flags,
                                            void *user_data) {
#if DEBUG_HEADERS
    std::cout << "Header: " << std::string((char *) name, namelen) << ": " << std::string((char *) value, valuelen)
              << std::endl;
#endif
    auto ssl_session = static_cast<HttpSession *>(user_data);
    int32_t stream_id = frame->hd.stream_id;
    try {
        ssl_session->setHeaders(stream_id, std::string((char *) name, namelen), std::string((char *) value, valuelen));
    } catch (StreamNotFound &e) {
#if DEBUG_HEADERS
        BOOST_LOG_TRIVIAL(error) << e.what();
#endif
    }

    return 0;
}

int
unit::server::callbacks::on_frame_recv_callback(nghttp2_session *session, const nghttp2_frame *frame, void *user_data) {
#if DEBUG_ON_FRAME
    std::cout << "Received frame: " << frame->hd.type << std::endl;
#endif
    if (frame->hd.flags == NGHTTP2_FLAG_END_STREAM) {
        int32_t stream_id = frame->hd.stream_id;

//        auto *user_session = static_cast<HttpSession *>(user_data);
//        user_session->setDataFinishedByStream(stream_id, true);
//        auto *res = new unit::server::data::response(R"({"test":"test"})", stream_id);
//        send_response(session, stream_id, res);
#if DEBUG_HEADERS
        std::cout << "Headers: " << '\n';
        for (auto &[key, value]: *user_session) {
            for (auto &[k, v]: value.headers) {
                std::cout << k << " : " << v << '\n';
            }
        }
#endif
#if DEBUG_FRAME_CALLBACK
        for (auto &[key, value]: *user_session) {
            std::cout << key << ": " << std::string((char *) value.data.data(), value.data.size()) << ", isFinished: "
                      << value.isFinished << '\n';
        }
        for (auto &[key, value]: *user_session) {
            for (auto &[k, v] : value.headers) {
                std::cout << "header: " << k << ", header value: " << v << '\n';
            }
        }
#endif
    }
#if 0
    if (frame->hd.type == NGHTTP2_HEADERS && frame->headers.cat == NGHTTP2_HCAT_REQUEST) {
    }
#endif
    return 0;
}

int unit::server::callbacks::on_data_chunk_recv_callback(nghttp2_session *session, uint8_t flags,
                                                         int32_t stream_id, const uint8_t *data,
                                                         size_t len, void *user_data) {
    auto *user_session = static_cast<HttpSession *>(user_data);
    user_session->push_stream_data(stream_id, (unsigned char *) data);
#if DEBUG_CHUNK_RECEIVED
    std::cout << "Received data chunk: " << std::string((char *) data, len) << std::endl;
#endif
    return 0;
}

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket, const std::string &key_file, const std::string &cert_file)
        : socket_(std::move(socket)), key_file_path(key_file), cert_file_path(cert_file) {
    nghttp2_session_callbacks *callbacks;
    nghttp2_session_callbacks_new(&callbacks);
    nghttp2_session_callbacks_set_send_callback(callbacks, unit::server::callbacks::send_callback);
    // TODO: callbacks
    nghttp2_session_callbacks_set_on_header_callback(callbacks, unit::server::callbacks::on_header_callback);
    nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, unit::server::callbacks::on_frame_recv_callback);
    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks,
                                                              unit::server::callbacks::on_data_chunk_recv_callback);
    nghttp2_session_server_new(&(this->session), callbacks, this);
    nghttp2_session_callbacks_del(callbacks);
}

HttpSession::~HttpSession() {
    nghttp2_session_del(this->session);
}

void HttpSession::start() {
    do_handshake();
}

void HttpSession::do_handshake() {
    auto self = shared_from_this();
    boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv13);
    create_ssl_ctx(ssl_context, this->key_file_path.c_str(), this->cert_file_path.c_str());
#if 0
    auto ssl_socket = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(std::move(this->socket_), ssl_context);
#endif
    this->ssl_socket = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
            std::move(this->socket_), ssl_context);
    ssl_socket->async_handshake(boost::asio::ssl::stream_base::server,
                                [this, self](const boost::system::error_code &error) {
                                    if (error) {
#if DEBUG_SSL_HANDSHAKE
                                        BOOST_LOG_TRIVIAL(error) << "SSL Handshake failed: " << error.message();
#endif
                                        return;
                                    }
#if DEBUG_SSL_HANDSHAKE
                                    BOOST_LOG_TRIVIAL(info) << "SSL Handshake successful.";
#endif
                                    // Print SSL details
                                    SSL *ssl = this->ssl_socket->native_handle();
#if DEBUG_SSL_HANDSHAKE
                                    std::cout << "SSL Version: " << SSL_get_version(ssl) << std::endl;
                                    std::cout << "SSL Cipher: " << SSL_get_cipher_name(ssl) << std::endl;
#endif
                                    // Get ALPN protocol to check for HTTP/2
                                    const unsigned char *alpn_protocol = nullptr;
                                    unsigned int alpn_len = 0;
                                    SSL_get0_alpn_selected(ssl, &alpn_protocol, &alpn_len);
                                    if (alpn_len == 2 && memcmp("h2", alpn_protocol, 2) == 0) {
#if DEBUG
                                        BOOST_LOG_TRIVIAL(info) << "ALPN selected protocol: HTTP/2";
#endif
                                    } else {
#if DEBUG
                                        BOOST_LOG_TRIVIAL(info) << "ALPN did not select HTTP/2. Selected: "
                                                                << std::string((const char *) alpn_protocol, alpn_len);
#endif
                                    }
                                    do_read();
                                });
}

void HttpSession::do_read() {
    auto self(shared_from_this());
    std::array<unsigned char, 8192> data{};
    this->ssl_socket->async_read_some(boost::asio::buffer(data),
                                      [this, self, &data](boost::system::error_code ec, std::size_t length) {
                                          if (!ec) {
#if DEBUG_ON_RECEIVE
                                              std::cout << "Data received: " << length << " bytes." << std::endl;
#endif
                                              ssize_t rv = nghttp2_session_mem_recv(session, (uint8_t *) data.data(),
                                                                                    length);
                                              if (rv < 0) {
#if DEBUG_ON_RECEIVE
                                                  BOOST_LOG_TRIVIAL(error) << "nghttp2_session_mem_recv failed: "
                                                            << nghttp2_strerror(rv);
#endif
                                                  return;
                                              }
                                              do_read();
                                          }
#ifdef DEBUG
                                          else {
                                              BOOST_LOG_TRIVIAL(error) << ec.what();
                                          }
#endif
                                      });
}

void HttpSession::push_stream_data(int32_t stream_id, unsigned char *data) {
    if (!this->full_data.contains(stream_id))
        this->full_data.emplace_hint(this->full_data.begin(), stream_id,
                                     unit::server::data::request{false, std::vector<unsigned char>(data, data + strlen(
                                             reinterpret_cast<const char *>(data))), stream_id});
    else
        this->full_data.at(stream_id).data.push_back(*data);
}

std::map<int32_t, unit::server::data::request>::const_iterator HttpSession::begin() const {
    return this->full_data.begin();
}

std::map<int32_t, unit::server::data::request>::const_iterator HttpSession::end() const {
    return this->full_data.end();
}

unit::server::data::request HttpSession::getUserDataByStream(int32_t stream_id) {
    try {
        return this->full_data.at(stream_id);
    } catch (std::out_of_range &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
        throw StreamNotFound("There is no stream with provided stream_id");
    }
}

void HttpSession::setDataFinishedByStream(int32_t stream_id, bool value) {
    try {
        this->full_data.at(stream_id).isFinished = value;
    } catch (std::out_of_range &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
        throw StreamNotFound("There is no stream with provided stream_id");
    }
}

void HttpSession::eraseData(int32_t stream) {
    this->full_data.erase(stream);
}

void HttpSession::setHeaders(int32_t stream_id, const std::string &header, const std::string &header_value) {
    try {
        this->pushEmptyUserData(stream_id);
        this->full_data.at(stream_id).headers[header] = header_value;
    } catch (std::out_of_range &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
        throw StreamNotFound("There is no stream with provided stream_id");
    }
}

void HttpSession::pushEmptyUserData(int32_t stream_id) {
    this->full_data.try_emplace(this->full_data.begin(), stream_id, unit::server::data::request(stream_id));
}

unit::server::data::response::response(int32_t stream_id) : stream_id(stream_id) {}

void unit::server::callbacks::send_response(nghttp2_session *session, int32_t stream_id,
                                            unit::server::data::response *data) {
    nghttp2_data_provider provider{};
    provider.source.fd = 0;
    provider.source.ptr = data;
    provider.read_callback = unit::server::callbacks::data_provider_callback;

    nghttp2_nv headers[] = {
            MAKE_NV(":status", "200"),
            MAKE_NV("content-type", "application/json")
            // Add other headers as necessary
    };

    // Submit the response
    int rv = nghttp2_submit_response(session, stream_id, headers,
                                     sizeof(headers) / sizeof(headers[0]), &provider);

    // Check rv for errors and handle appropriately
    // Assuming rv == 0 for successful submission
    if (rv == 0) {
        // Send the response to the client
        nghttp2_session_send(session);
    } else {
        BOOST_LOG_TRIVIAL(error) << nghttp2_strerror(rv);
    }
}

ssize_t unit::server::callbacks::data_provider_callback(nghttp2_session *session, int32_t stream_id,
                                                        uint8_t *buf, size_t length, uint32_t *data_flags,
                                                        nghttp2_data_source *source, void *user_data) {
    auto *data = static_cast<unit::server::data::response *>(user_data);  // Cast user_data back to the expected type
    size_t data_len = data->json_response.size();  // Calculate the length of the data

    // Determine the amount of data to send in this callback invocation
    size_t copy_len = data_len < length ? data_len : length;

    // Copy the data from the user_data to the provided buffer
    memcpy(buf, data->json_response.c_str(), copy_len);

    // Decrease remaining length of the data
    data += copy_len;
    data_len -= copy_len;

    // Save the new pointer position and the remaining data length back into user_data
    user_data = data;

    // If there is no more data to send, set the NGHTTP2_DATA_FLAG_EOF flag
    if (data_len == 0) {
        *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    }
    // Return the number of bytes we have copied into buf
    return static_cast<ssize_t>(copy_len);
}

static ssize_t unit::server::callbacks::send_callback(nghttp2_session *session, const uint8_t *data,
                                                      size_t length, int flags, void *user_data) {
    auto *session_data = static_cast<HttpSession *>(user_data);
    BOOST_LOG_TRIVIAL(info) << data;
    session_data->ssl_socket->async_write_some(boost::asio::buffer(data, strlen(reinterpret_cast<const char *>(data))), unit::server::callbacks::write_handler);
    return (ssize_t) length;
}

void unit::server::callbacks::write_handler(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        BOOST_LOG_TRIVIAL(info) << "Successfully wrote " << bytes_transferred << " bytes.";
    } else {
        BOOST_LOG_TRIVIAL(error) << "Write failed: " << error.message();
    }
}