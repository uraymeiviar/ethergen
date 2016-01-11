#include "cpuworker.hpp"
#include <memory>
#include "sha3.hpp"

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
        this->workerThread = std::thread(&CpuWorker::threadProc,this);
    }
    else if(this->running == true && run == false)
    {
        this->running = false;
        if(this->workerThread.joinable())
        {
            this->workerThread.join();
        }
    }
}

void CpuWorker::setWork(const Work& work, uint64_t startNonce)
{
    std::lock_guard<std::mutex> lock(this->workerRunMutex);
    Worker::setWork(work,startNonce);
    this->numPages = this->workGraph->getDAGByteLength() / 128;
}

void CpuWorker::findNonce()
{
    //std::lock_guard<std::mutex> lock(this->workerRunMutex);
    this->work.checkNonce(this->getCurrentNonce(), this->workResult, this->workGraph );
    
    if(this->workResult.hash < this->bestResult)
    {
        this->bestResult = this->workResult.hash;
    }
    
    if(this->workResult.hash < this->work.target)
    {
        this->onValidResult(*this,this->workResult);
    }
    this->updateNextNonce();
}

/*
void CpuWorker::findNonce()
{
    memcpy(this->smix[0].ptr(), this->work.headerHash.ptr(), 32);
    memcpy(this->smix[0].ptr()+32, &this->workResult.nonce, 8);
    
    SHA3_512(this->smix[0].ptr(), this->smix[0].ptr(), 40);
    
    memcpy(this->smix[1].ptr(), this->smix[0].ptr(), 64);
    memcpy(this->smix[2].ptr(), this->smix[0].ptr(), 64);
    
    uint32_t* mix = (uint32_t*)this->smix[1].ptr();
    for(uint32_t i=0 ; i<64 ; i++)
    {
        uint32_t index = ((smix[0].value32(0) ^ i) * 0x01000193 ^ mix[i % 32]) % this->numPages;
        
        const uint32_t* dagNode = this->workGraph->getDAG()[2 * index ].ptr32();
        for (size_t w = 0; w < 32; w++)
        {
            mix[w] = (mix[w] * 0x01000193 ^ dagNode[w]);
        }
    }
    
    uint32_t* mixhashPtr = (uint32_t*)this->workResult.mixHash.ptr();
    for (size_t w = 0; w < 8; w++)
    {
        mixhashPtr[w] = mix[w*4 + 0]  * 0x01000193 ^ mix[w*4 + 1];
        mixhashPtr[w] = mixhashPtr[w] * 0x01000193 ^ mix[w*4 + 2];
        mixhashPtr[w] = mixhashPtr[w] * 0x01000193 ^ mix[w*4 + 3];
    }
    
    SHA3_256(this->workResult.hash.ptr(), smix[0].ptr(), 64 + 32);
    
    if(this->workResult.hash < this->bestResult)
    {
        this->bestResult = this->workResult.hash;
    }
    
    if(this->workResult.hash < this->work.target)
    {
        this->onValidResult(*this,this->workResult);
    }
    this->workResult.nonce++;
}
*/