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
    Miner(Bits<160> accountId);
    virtual ~Miner();
    void addWorker(WorkerFactory& workerFactory, ConnectionFactory& connectionFactory, std::string name, std::string endpoint);
    size_t countWorker() const;
    const Worker* getWorker(size_t index) const;
    const Connection* getWorkerConnection(size_t index) const;
    void destroyWorker(size_t index);
    const Bits<160>& getAccountId() const;
    virtual void setRun(bool run);
    bool isRunning() const;
    uint64_t getCurrentHashrate() const;
protected:
    bool running;
    Bits<160> accountId;
    std::vector<std::pair<std::unique_ptr<Worker>,std::unique_ptr<Connection>>> workers;
    void eventOnNewWork(Connection& connection, const Work& work, uint64_t startNonce,Worker* worker);
    void eventWorkResult(const Worker& worker, WorkResult result, Connection* connection);
};

#endif /* miner_h */
