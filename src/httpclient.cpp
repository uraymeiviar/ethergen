#include "httpclient.h"
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <iostream>

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif

std::map<std::string,uint32_t> HTTPEndpoint::hostnameCache;

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
    std::lock_guard<std::mutex> lock(this->lock);
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
            request += "Content-Length: "+std::to_string(body.length())+"\r\n";
            request += "User-Agent: ethergen-http\r\n";
            request += "Pragma: no-cache\r\n";
            request += "Cache-Control: no-cache\r\n";
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
    size_t pathPos = hostPos;
    if(protoPos == std::string::npos)
    {
        protoPos = 0;
        hostPos = 0;
    }
    
    size_t portPos = url.find_first_of(":",hostPos);
    if(portPos == std::string::npos)
    {
        this->port = 80;
        pathPos = url.find_first_of("/",hostPos);
        portPos = pathPos;
    }
    else
    {
        pathPos = url.find_first_of("/",portPos+1);
        std::string portStr = url.substr(portPos+1,pathPos-(portPos+1));
        this->port = std::stoi(portStr);
    }
    
    this->hostname = url.substr(hostPos,portPos-hostPos);
    if(pathPos == std::string::npos)
    {
        this->path = "/";
    }
    else
    {
        this->path = url.substr(pathPos,url.length()-pathPos);
    }
}

HTTPEndpoint::HTTPEndpoint(std::string hostname, uint16_t port, std::string path)
{
    this->hostname = hostname;
    this->port = port;
    this->path = path;
}

bool HTTPEndpoint::getIPEndpoint(struct sockaddr_in& ipEndpoint) const
{
    if(HTTPEndpoint::hostnameCache.find(this->hostname) != HTTPEndpoint::hostnameCache.end())
    {
        ipEndpoint.sin_addr.s_addr = HTTPEndpoint::hostnameCache[this->hostname];
        ipEndpoint.sin_family = AF_INET;
        ipEndpoint.sin_port = htons(this->port);
        return true;
    }
    else
    {
        std::cout << "resolving host name " << this->hostname << std::endl;
        hostent* host = gethostbyname(this->hostname.c_str());
        if(host != nullptr)
        {
            ipEndpoint.sin_addr.s_addr = *(uint32_t *)(host->h_addr_list[0]);
            ipEndpoint.sin_family = AF_INET;
            ipEndpoint.sin_port = htons(this->port);
            
            std::cout << "endpoint : " << inet_ntoa(ipEndpoint.sin_addr) << " port " << std::to_string(this->port) << std::endl;
            
            HTTPEndpoint::hostnameCache.insert(std::make_pair(this->hostname, ipEndpoint.sin_addr.s_addr));
            return true;
        }
        else
        {
            return false;
        }
    }
}