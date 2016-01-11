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

void CpuWorker::threadProc()
{
    while(this->running)
    {
        this->findNonce();
    }
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

static inline uint32_t fnv(uint32_t a, uint32_t b)
{
    return (a + (a<<1) + (a<<4) + (a<<7) + (a<<8) + (a<<24)) ^ b;
}

inline void CpuWorker::findNonce()
{
    uint64_t nonce = this->currentNonce.fetch_add(1, std::memory_order_relaxed);
    this->totalNonce++;
    
    Bytes<64> smix[3];
    uint32_t* mix = (uint32_t*)smix[1].ptr();
    const Bytes<64>* dag = this->workGraph->getDAG();
    
    memcpy(smix[0].ptr(), this->work.headerHash.ptr(), 32);
    memcpy(smix[0].ptr()+32, &nonce, 8);
    
    SHA3_512(smix[0].ptr(), smix[0].ptr(), 40);
    
    memcpy(smix[1].ptr(), smix[0].ptr(), 64);
    memcpy(smix[2].ptr(), smix[0].ptr(), 64);
    
    for (uint32_t i = 0; i != 64; ++i)
    {
        uint32_t index = fnv(smix[0].value32(0)^i, mix[i % 32]) % this->numPages;
        
        const uint32_t* dagNode = dag[index*2 ].ptr32();
        for (size_t w = 0; w < 32; w++)
        {
            mix[w] = fnv(mix[w],dagNode[w]);
        }
    }
    
    for (size_t w = 0; w < 8; w++)
    {
        mix[w] = fnv(mix[w*4],mix[w*4 + 1]);
        mix[w] = fnv(mix[w  ],mix[w*4 + 2]);
        mix[w] = fnv(mix[w  ],mix[w*4 + 3]);
    }
    
    Bits<256> hash;
    SHA3_256(hash.ptr(), smix[0].ptr(), 64 + 32);
    
    if(hash < this->bestResult)
    {
        this->bestResult = hash;
    }
    
    if(hash < this->work.target)
    {
        this->workResult.nonce = nonce;
        this->workResult.hash = hash;
        this->workResult.mixHash.copyFrom((uint8_t*)mix);
        this->onValidResult(*this,this->workResult);
    }
}
