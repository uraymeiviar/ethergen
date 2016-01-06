#ifndef connection_h
#define connection_h

#include <functional>
#include <string>
#include "bits.hpp"

class Connection
{
public:
    void setWork(const Work& work, uint64_t startNonce = 0);
    virtual void requestNewWork() const = 0;
    virtual void submitResult(const WorkResult& result) = 0;
    virtual void setEndPoint(std::string endPoint) = 0;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual void setAccountId(const Bits<160>& account);
    virtual void setWorkerName(std::string workerName);
    std::string getEndPoint() const;
    std::function<void(Connection&,const Work&)> onNewWork;
    std::function<void(Connection&)> onConnected;
    std::function<void(Connection&)> onDisconnected;
protected:
    std::string endPoint;
};

class ConnectionFactory
{
public:
    virtual std::unique_ptr<Connection> create(const Bits<160>& account, std::string workerName) = 0;
};

#endif /* connection_h */
