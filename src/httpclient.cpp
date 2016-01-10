#include "httpclient.h"
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif

HTTPClient::HTTPClient()
{
    this->tcpSocket = 0;
}

HTTPClient::~HTTPClient()
{
    if(this->tcpSocket != 0)
    {
        shutdown(this->tcpSocket,SHUT_RDWR);
        close(this->tcpSocket);
        this->tcpSocket = 0;
    }
}

void HTTPClient::initSocket()
{
    this->readBuffer[HTTPClient::readBufferSize-1] = 0;
    this->tcpSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct timeval tv;
    tv.tv_sec =  10;  /* 15 Secs Timeout */
    tv.tv_usec = 0;
    setsockopt(this->tcpSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
}

std::string HTTPClient::http(std::string method,std::string url,std::string body)
{
    std::string result;
    HTTPEndpoint httpEndpoint(url);
    struct sockaddr_in ipEndpoint;
    if(httpEndpoint.getIPEndpoint(ipEndpoint))
    {
        this->initSocket();
        if(connect(this->tcpSocket, (struct sockaddr*)&ipEndpoint, sizeof(struct sockaddr_in)) != -1)
        {
            std::string request = method+" "+httpEndpoint.path+" HTTP/1.0\r\n";
            request += "Connection: close\r\n";
            request += "Content-Type: application/json\r\n";
            request += "\r\n";
            request += body;
            
            if(send(this->tcpSocket, request.c_str(), request.length(),MSG_NOSIGNAL) > 0)
            {
                size_t totalRead = 0;
                int bytesRead = 0;
                do
                {
                    if(HTTPClient::readBufferSize-1 > totalRead)
                    {
                        bytesRead = (int)read(this->tcpSocket, &this->readBuffer[totalRead], HTTPClient::readBufferSize-totalRead);
                        totalRead += bytesRead;
                        readBuffer[totalRead] = 0;
                        char* find = NULL;
                        if(bytesRead > 0)
                        {
                            find = strstr(this->readBuffer, "\r\n\r\n");
                        }
                        
                        if(find != NULL)
                        {
                            result = std::string(find+4);
                            shutdown(this->tcpSocket,SHUT_WR);
                        }
                    }
                    else
                    {
                        this->readBuffer[HTTPClient::readBufferSize-1] = 0;
                        shutdown(this->tcpSocket,SHUT_RDWR);
                        bytesRead = 0;
                    }
                }
                while(bytesRead > 0);
            }
        }
        close(this->tcpSocket);
        this->tcpSocket = 0;
    }
    return result;
}

std::string HTTPClient::httpPost(std::string url,std::string body)
{
    return this->http("POST",url,body);
}

std::string HTTPClient::httpGet(std::string url)
{
    return this->http("GET",url,"");
}

HTTPEndpoint::HTTPEndpoint(std::string url)
{
    size_t hostPos = std::string("http://").length();
    size_t protoPos = url.find_first_of("http://");
    if(protoPos == std::string::npos)
    {
        protoPos = 0;
        hostPos = 0;
    }
    size_t portPos = url.find_first_of(":",hostPos);
    if(portPos == std::string::npos)
    {
        portPos = protoPos;
    }
    size_t pathPos = url.find_first_of("/",portPos);
    
    std::string portStr = url.substr(portPos,pathPos-portPos);
    this->port = std::stoi(portStr);
    this->hostname = url.substr(hostPos,portPos-protoPos);
    this->path = url.substr(pathPos,url.length()-pathPos);
}

HTTPEndpoint::HTTPEndpoint(std::string hostname, uint16_t port, std::string path)
{
    this->hostname = hostname;
    this->port = port;
    this->path = path;
}

bool HTTPEndpoint::getIPEndpoint(struct sockaddr_in& ipEndpoint) const
{
    hostent* host = gethostbyname(hostname.c_str());
    if(host != nullptr)
    {
        ipEndpoint.sin_addr.s_addr = inet_addr(host->h_addr_list[0]);
        ipEndpoint.sin_family = AF_INET;
        ipEndpoint.sin_port = htons(this->port);
        return true;
    }
    else
    {
        return false;
    }
}