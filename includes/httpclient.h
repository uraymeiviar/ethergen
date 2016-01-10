#ifndef httpclient_h
#define httpclient_h
#include <string>
#include <netinet/in.h>
#include <map>

class HTTPEndpoint
{
public:
    HTTPEndpoint(std::string url);
    HTTPEndpoint(std::string hostname, uint16_t port, std::string path);
    std::string hostname;
    std::string path;
    uint16_t port;
    bool getIPEndpoint(struct sockaddr_in& ipEndPoint) const;
    static std::map<std::string,uint32_t> hostnameCache;
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
