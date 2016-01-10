#ifndef poolconnection_h
#define poolconnection_h
#include "connection.hpp"

class PoolConnectionFactory : public ConnectionFactory
{
public:
    std::unique_ptr<Connection> create(const Bits<160>& account, std::string workerName) override;
};

class PoolConnection : public Connection
{
public :
    PoolConnection(const Bits<160>& account, std::string workerName);
    void requestNewWork() override;
    void submitResult(const WorkResult& result) override;
    static PoolConnectionFactory& getFactory();
protected:
    std::string getPoolUrl() const;
};

#endif /* poolconnection_h */
