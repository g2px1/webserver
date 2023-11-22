#include "handler/Server.h"
#include "request/requestTypesStr.h"


std::string getFileExtension(const std::string& fileName) {
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos) {
        return fileName.substr(dotPos + 1);
    }
    return ""; // Return empty string if no extension found
}

long getFileSize(const std::string& fileName) {
    std::ifstream file(fileName, std::ifstream::ate | std::ifstream::binary);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << fileName << std::endl;
        return -1; // Return -1 or throw an exception if file can't be opened
    }
    return file.tellg();
}

int main() {
    unit::server::handler::Server server("/Users/kirillzhukov/Desktop/config.toml");
    server.handle(unit::server::request::GET, R"(^\/.*\.[a-zA-Z0-9]+$)",
                  [&](unit::server::data::HttpRequest&request, unit::server::data::HttpResponse&response) {
                      std::string path = server.config.getKey("doc_root").as_string()->get() + request.headers.at(":path");
                      long size = getFileSize(path);
                      std::string extension = "application/";
                      extension.append(getFileExtension(path) == "js" ? "javascript" : "css");
                      char buffer[20];
                      snprintf(buffer, sizeof(buffer), "%ld", size);
                      char* myLongAsString = buffer;
                      response.writeFile(path);
                      response.addHeader((char *) ":status", (char *) "200");
                      response.addHeader((char *) "Content-Type", (char *) extension.c_str());
                      response.addHeader((char *) "Content-Length", myLongAsString);
                      response.addHeader((char *) "Cache-Control", (char *) "no-store, no-cache, must-revalidate, proxy-revalidate, max-age=0");
                      response.addHeader((char *) "Access-Control-Allow-Origin", (char*) "*");
                      // response.addHeader((char *) "Keep-Alive", (char*) "timeout=100000");
                      path.clear();
                  });
    server.handle(unit::server::request::GET, "/favicon.ico", [&](unit::server::data::HttpRequest&request, unit::server::data::HttpResponse&response) {
        response.writeFile("/Users/kirillzhukov/Downloads/favicon.ico");
        response.addHeader((char *) ":status", (char *) "200");
    });
    server.handle(unit::server::request::GET, R"(/)",
                  [&](unit::server::data::HttpRequest&request, unit::server::data::HttpResponse&response) {
                      // response.writeFile("/Users/kirillzhukov/Desktop/test.html");
                      response.writeFile("/Users/kirillzhukov/Desktop/webtest/test.html");
                      // response.writeFile("/Users/kirillzhukov/CLionProjects/moexServer/websiteTest/index.html");
                      response.addHeader((char *) ":status", (char *) "200");
                      response.addHeader((char *) "Content-Type", (char *) "text/html; charset=utf-8");
                      // response.addHeader((char *) "Keep-Alive", (char*) "timeout=100000");
                  });
    server.start();

    return 0;
}
