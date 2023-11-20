cmake_minimum_required(VERSION 3.26)
project(tcpSocketTest)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS json log url)
find_package(OpenSSL REQUIRED)

include_directories(
        utils
        server
        session
)

add_definitions(
        -DDEBUG=0
        -DDEBUG_HEADERS=0
        -DDEBUG_SSL_HANDSHAKE=0
        -DDEBUG_ON_RECEIVE=0
        -DDEBUG_ON_FRAME=0
        -DDEBUG_FRAME_CALLBACK=0
        -DDEBUG_CHUNK_RECEIVED=0
        -DDEBUG_SOCKET_WRITING=0
        -DDEBUG_RESPONSE_EMULATION=1
        -DDEBUG_STREAM_PRINTING=0
        -DDEBUG_CONNECTION=0
)

add_executable(tcpSocketTest main.cpp
        session/HttpSession.cpp
        session/HttpSession.h
        session/SessionManager.cpp
        session/SessionManager.h
        utils/ssl/CTX_util.h
        utils/toml/toml.hpp
        session/HttpSessionException.cpp
        session/HttpSessionException.h
        server/data/HttpRequest.cpp
        server/data/HttpRequest.h
        server/data/TestHttpResponse.cpp
        server/data/TestHttpResponse.h
        server/data/HttpResponse.cpp
        server/data/HttpResponse.h
        server/data/json.cpp
        server/handler/Server.cpp
        server/handler/Server.h
        utils/toml/toml.cpp
        utils/configuration/ConfigReader.cpp
        utils/configuration/ConfigReader.h
        utils/path/pathUtil.h
        server/handler/EndpointHandler.h
        server/handler/BasicEndpointHandler.cpp
        server/handler/BasicEndpointHandler.h
        server/data/types.h
        utils/request/requestTypesStr.h
        utils/request/requestTypeStr.cpp
)

target_link_libraries(${PROJECT_NAME}
        OpenSSL::SSL
        Boost::json
        Boost::boost
        Boost::log
        Boost::url
        /usr/local/lib/libnghttp2.a
)