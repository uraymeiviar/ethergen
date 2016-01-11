#ifndef miner_h
#define miner_h

#include <string>
#include <memory>
#include <vector>
#include <stdint.h>

#include "bits.hpp"
#include "connection.hpp"
#include "worker.hpp"

class Miner
{
public:
    Miner();
    virtual ~Miner();
    void addWorker(WorkerFactory& workerFactory,
                   ConnectionFactory& connectionFactory,
                   std::string name,
                   std::string endpoint,
                   Bits<160> accountId,
                   std::string workerParam = "");
    size_t countWorker() const;
    const Worker* getWorker(size_t index) const;
    const Connection* getWorkerConnection(size_t index) const;
    void destroyWorker(size_t index);
    const Bits<160>& getAccountId() const;
    virtual void setRun(bool run);
    bool isRunning() const;
    uint64_t getCurrentHashrate() const;
    Bits<256> getBestResult() const;
    void update();
protected:
    bool running;
    std::vector<std::pair<std::unique_ptr<Worker>,std::unique_ptr<Connection>>> workers;
    void eventOnNewWork(Connection& connection, const Work& work, uint64_t startNonce,Worker* worker);
    void eventOnInitWork(Connection& connection, const Work& work, uint64_t startNonce,Worker* worker);
    void eventWorkResult(const Worker& worker, WorkResult result, Connection* connection);
    void eventNewTarget(Connection& connection,const Bits<256>& target,Worker* worker);
};

#endif /* miner_h */
