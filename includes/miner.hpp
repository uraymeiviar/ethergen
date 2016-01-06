#ifndef miner_h
#define miner_h

#include <string>
#include <stdint.h>

#include "bits.hpp"
#include "connection.hpp"
#include "worker.hpp"

class Miner
{
public:
    Miner(Bits<160> accountId);
    void addWorker(WorkerFactory& workerFactory, ConnectionFactory& connectionFactory, std::string name);
    size_t countWorker() const;
    const Worker& getWorker(size_t index) const;
    const Connection& getWorkerConnection(size_t index) const;
    void destroyWorker(size_t index);
    const Bits<160>& getAccountId() const;
    void setRun(bool run);
    bool isRunning() const;
    uint64_t getCurrentHashrate() const;
protected:
    Bits<160> accountId;
    std::vector<std::pair<Worker,Connection>> workers;
    void eventOnNewWork(Connection& connection, const Work& work);
    void eventOnNetConnected(Connection& connection);
    void eventOnNetDisconnected(Connection& connection);
};

#endif /* miner_h */
