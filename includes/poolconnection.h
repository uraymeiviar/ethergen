#ifndef poolconnection_h
#define poolconnection_h
#include "connection.hpp"
#include "httpclient.h"

class PoolConnectionFactory : public ConnectionFactory
{
public:
    std::unique_ptr<Connection> create(const Bits<160>& account, std::string workerName, std::string endpoint) override;
};

class PoolConnection : public Connection
{
public :
    PoolConnection(const Bits<160>& account, std::string workerName, std::string endpoint);
    void requestNewWork() override;
    uint64_t getBlockNumber() override;
    void submitResult(const WorkResult& result) override;
    static PoolConnectionFactory& getFactory();
protected:
    std::string getPoolUrl() const;
    HTTPClient httpClient;
    std::string httpBodyGetWork;
    std::string httpBodyGetBlockNum;
    
    Work lastWork;
};

#endif /* poolconnection_h */
