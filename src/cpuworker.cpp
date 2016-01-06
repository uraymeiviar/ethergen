#include "cpuworker.hpp"
#include <memory>

std::unique_ptr<Worker> CpuWorkerFactory::create(std::string name)
{
    return std::make_unique<CpuWorker>();
}

CpuWorkerFactory& CpuWorker::getFactory()
{
    static CpuWorkerFactory factory;
    return factory;
}

CpuWorker::CpuWorker()
: Worker()
{
    this->workerThread = std::thread(&CpuWorker::threadProc,this);
}

CpuWorker::~CpuWorker()
{
    std::unique_lock<std::mutex> pausedSignal(this->workerPauseMutex);
    this->setRun(false);
    this->workerPausedSignal.wait(pausedSignal);
    
    if(this->workerThread.joinable())
    {
        std::unique_lock<std::mutex> runSignal(this->workerRunMutex);
        this->workerRunSignal.notify_all();
        runSignal.unlock();
        this->workerThread.join();
    }
}

void CpuWorker::threadProc()
{
    bool quit = false;
    while(!quit)
    {
        if(!this->running)
        {
            std::unique_lock<std::mutex> pausedSignal(this->workerPauseMutex);
            
            std::unique_lock<std::mutex> runSignal(this->workerRunMutex);
            this->workerPausedSignal.notify_all();
            pausedSignal.unlock();
            
            this->workerRunSignal.wait(runSignal);
            if(!this->running)
            {
                quit = true;
            }
        }
        else
        {
            this->findNonce();
        }
    }
}

void CpuWorker::setRun(bool run)
{
    if(this->running == false && run == true)
    {
        std::lock_guard<std::mutex> runSignal(this->workerRunMutex);
        this->running = true;
        this->workerRunSignal.notify_all();
    }
    else if(this->running == true && run == false)
    {
        std::unique_lock<std::mutex> pausedSignal(this->workerPauseMutex);
        this->running = false;
        this->workerPausedSignal.wait(pausedSignal);
    }
}

void CpuWorker::findNonce()
{
    
}