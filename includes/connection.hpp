#ifndef connection_h
#define connection_h

#include <functional>
#include <string>
#include "bits.hpp"
#include "work.hpp"

class Connection
{
public:
    void setWork(const Work& work, uint64_t startNonce);
    virtual void requestNewWork() = 0;
    virtual uint64_t getBlockNumber() = 0;
    virtual void submitResult(const WorkResult& result) = 0;
    virtual void setEndPoint(std::string endPoint);
    virtual void setAccountId(const Bits<160>& account);
    virtual void setWorkerName(std::string workerName);
    virtual const Bits<160>& getAccountId() const;
    virtual std::string getWorkerName() const;
    std::string getEndPoint() const;
    std::function<void(Connection&,const Work&, uint64_t startNonce)> onNewWork;
    virtual ~Connection();
protected:
    std::string endPoint;
    std::string workerName;
    Bits<160> accountId;
};

class ConnectionFactory
{
public:
    virtual std::unique_ptr<Connection> create(const Bits<160>& account, std::string workerName, std::string endpoint) = 0;
};

#endif /* connection_h */
