#ifndef worker_h
#define worker_h

#include <string>
#include <memory>
#include <functional>
#include <stdint.h>
#include <chrono>
#include "work.hpp"

class Worker
{
public:
    virtual void setParam(std::string param);
    virtual void setWork(const Work& work, uint64_t startNonce = 0);
    virtual void setTarget(const Bits<256>& target);
    const Work& getCurrentWork() const;
    virtual void setRun(bool run);
    virtual bool isRunning() const;
    virtual uint64_t getCurrentHashrate() const;
    virtual void updateHashrate() = 0;
    Worker();
    virtual ~Worker();
    std::string getName() const;
    Bits<256> getIdentity() const;
    Bits<256> getBestResult() const;
    std::function<void(const Worker&, WorkResult)> onValidResult;
protected:
    std::chrono::system_clock::time_point lastNonceCheckTime;
    std::string name;
    std::string param;
    Bits<256> identity;
    Bits<256> bestResult;
    bool running;
    uint64_t currentHashrate;
    Work work;
    WorkResult workResult;
    std::shared_ptr<WorkGraph> workGraph;
};

class WorkerFactory
{
public:
    virtual std::unique_ptr<Worker> create(std::string name) = 0;
};

#endif /* worker_h */
