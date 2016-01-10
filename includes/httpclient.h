#ifndef httpclient_h
#define httpclient_h
#include <string>
#include <netinet/in.h>

class HTTPEndpoint
{
public:
    HTTPEndpoint(std::string url);
    HTTPEndpoint(std::string hostname, uint16_t port, std::string path);
    std::string hostname;
    std::string path;
    uint16_t port;
    bool getIPEndpoint(struct sockaddr_in& ipEndPoint) const;
protected:
    struct sockaddr_in ipEndpoint;
};

class HTTPClient
{
public:
    static const size_t readBufferSize = 2048;
    HTTPClient();
    ~HTTPClient();
    std::string http(std::string method,std::string url,std::string body);
    std::string httpPost(std::string url,std::string body);
    std::string httpGet(std::string url);
protected:
    void initSocket();
    int tcpSocket;
    char readBuffer[readBufferSize];
};

#endif /* httpclient_h */
