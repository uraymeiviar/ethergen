#ifndef worker_h
#define worker_h

#include <string>
#include <memory>
#include <functional>
#include <stdint.h>
#include "work.hpp"

class Worker
{
public:
    virtual void setWork(const Work& work, uint64_t startNonce = 0);
    const Work& getCurrentWork() const;
    virtual void setRun(bool run);
    virtual bool isRunning() const;
    virtual uint64_t getCurrentNonce() const;
    virtual uint64_t getCurrentHashrate() const;
    Worker();
    virtual ~Worker();
    std::string name;
    std::function<void(const Worker&, WorkResult)> onValidResult;
protected:
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
