#include "cpuworker.hpp"
#include <memory>
#include "sha3.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

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
    this->threadCount = 1;
    this->workerThreads.resize(1);
}

CpuWorker::~CpuWorker()
{
    this->setRun(false);
}

void CpuWorker::setRun(bool run)
{
    if(this->running == false && run == true)
    {
        this->running = true;
        for(size_t i=0; i<this->workerThreads.size() ; i++)
        {
            this->workerThreads[i] = std::thread(&CpuWorker::threadProc,this);
        }
    }
    else if(this->running == true && run == false)
    {
        this->running = false;
        for(size_t i=0; i<this->workerThreads.size() ; i++)
        {
            if(this->workerThreads[i].joinable())
            {
                this->workerThreads[i].join();
            }
        }
    }
}

void CpuWorker::updateHashrate()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::duration<float> deltaSecs = std::chrono::duration_cast<std::chrono::duration<float>>(now - this->lastNonceCheckTime);
    this->currentHashrate = this->totalNonce/deltaSecs.count();
    this->lastNonceCheckTime = now;
    this->totalNonce = 0;
}

void CpuWorker::setWork(const Work& work, uint64_t startNonce)
{
    std::lock_guard<std::mutex> lock(this->setWorkMutex);
    Worker::setWork(work,startNonce);
    this->currentNonce = startNonce;
    this->numPages = this->workGraph->getDAGByteLength() / 128;
}

void CpuWorker::setParam(std::string param)
{
    Worker::setParam(param);
    rapidjson::Document configJson;
    rapidjson::ParseResult parseResult = configJson.Parse(param.c_str());
    if(parseResult)
    {
        if(configJson.HasMember("threads") && configJson["threads"].IsInt())
        {
            this->threadCount = configJson["threads"].GetInt();
            bool lastState = this->isRunning();
            this->setRun(false);
            this->workerThreads.resize(this->threadCount);
            this->setRun(lastState);
        }
    }
}

void CpuWorker::threadProc()
{
    HashPad hashPad;
    while(this->running)
    {
        hashPad.init(this->currentNonce.fetch_add(1, std::memory_order_relaxed),
                     this->work.headerHash);
        hashPad.hash(this->workGraph);
        this->totalNonce++;
        
        if(hashPad.isHashResultLowerThan(this->bestResult))
        {
            this->bestResult.copyFrom(hashPad.getHashResultPtr());
        }
        
        if(hashPad.isHashResultLowerThan(this->work.target))
        {
            this->workResult.nonce = hashPad.getNonce();
            this->workResult.hash.copyFrom(hashPad.getHashResultPtr());
            this->workResult.mixHash.copyFrom(hashPad.getMixResultPtr());
            this->onValidResult(*this,this->workResult);
        }
    }
}
