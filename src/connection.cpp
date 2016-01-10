#include "connection.hpp"

void Connection::setEndPoint(std::string endPoint)
{
    this->endPoint = endPoint;
}

std::string Connection::getEndPoint() const
{
    return this->endPoint;
}

void Connection::setWork(const Work& work, uint64_t startNonce)
{
    if(this->onNewWork)
    {
        this->onNewWork(*this,work, startNonce);
    }
}

void Connection::setAccountId(const Bits<160>& account)
{
    this->accountId = account;
}

void Connection::setWorkerName(std::string workerName)
{
    this->workerName = workerName;
}

const Bits<160>& Connection::getAccountId() const
{
    return this->accountId;
}

std::string Connection::getWorkerName() const
{
    return this->workerName;
}

Connection::~Connection()
{

}