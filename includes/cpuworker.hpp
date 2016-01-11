#ifndef cpuworker_hpp
#define cpuworker_hpp

#include "worker.hpp"
#include <thread>
#include <atomic>

class CpuWorkerFactory : public WorkerFactory
{
public:
    virtual std::unique_ptr<Worker> create(std::string name) override;
};

class CpuWorker : public Worker
{
public:
    void setWork(const Work& work, uint64_t startNonce ) override;
    void setParam(std::string param) override;
    static CpuWorkerFactory& getFactory();
    virtual void setRun(bool run) override;
    virtual void updateHashrate() override;
    CpuWorker();
    virtual ~CpuWorker() override;
protected:
    std::atomic<uint64_t> totalNonce;
    std::atomic<uint64_t> currentNonce;
    void threadProc();
    inline void findNonce();
    int threadCount;
    std::vector<std::thread> workerThreads;
    std::mutex setWorkMutex;
    std::array<Bytes<64>,3> smix;
    size_t numPages;
};

#endif
