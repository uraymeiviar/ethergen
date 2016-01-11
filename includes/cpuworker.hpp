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
    void setWork(const Work& work, uint64_t startNonce ) override;
    static CpuWorkerFactory& getFactory();
    virtual void setRun(bool run) override;
    CpuWorker();
    virtual ~CpuWorker() override;
protected:
    void threadProc();
    void findNonce();
    std::thread workerThread;
    std::mutex workerRunMutex;
    std::array<Bytes<64>,3> smix;
    size_t numPages;
};

#endif
