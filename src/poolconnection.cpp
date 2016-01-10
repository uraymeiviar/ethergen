#include "poolconnection.h"

PoolConnectionFactory& PoolConnection::getFactory()
{
    static PoolConnectionFactory factory;
    return factory;
}

std::unique_ptr<Connection> PoolConnectionFactory::create(const Bits<160>& account, std::string workerName)
{
    return std::make_unique<PoolConnection>(account,workerName);
}

PoolConnection::PoolConnection(const Bits<160>& account, std::string workerName)
{
    this->setAccountId(account);
    this->setWorkerName(workerName);
}

std::string PoolConnection::getPoolUrl() const
{
    std::string url = this->endPoint;
    std::string accountToken = "<account>";
    size_t accountTokenPos = url.find(accountToken);
    if(accountTokenPos != std::string::npos)
    {
        url.replace(accountTokenPos,accountToken.length(),this->accountId.toString());
    }
    
    std::string workerToken = "<worker>";
    size_t workerTokenPos = url.find(workerToken);
    if(workerTokenPos != std::string::npos)
    {
        url.replace(workerTokenPos, workerToken.length(), this->workerName);
    }
    return url;
}

void PoolConnection::requestNewWork()
{
    
}

void PoolConnection::submitResult(const WorkResult& result)
{
    
}