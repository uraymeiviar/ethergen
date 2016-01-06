#ifndef cpuworker_hpp
#define cpuworker_hpp

#include "worker.hpp"
#include <thread>

class CpuWorkerFactory : public WorkerFactory
{
public:
    virtual std::unique_ptr<Worker> create(std::string name) override;
};

class CpuWorker : public Worker
{
public:
    static CpuWorkerFactory& getFactory();
    virtual void setRun(bool run) override;
    CpuWorker();
    virtual ~CpuWorker() override;
protected:
    void threadProc();
    void findNonce();
    std::thread workerThread;
    
    std::mutex workerRunMutex;
    std::condition_variable workerRunSignal;
    
    std::mutex workerPauseMutex;
    std::condition_variable workerPausedSignal;
};

#endif
