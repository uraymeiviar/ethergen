#ifndef cpuworker_hpp
#define cpuworker_hpp

#include "worker.hpp"
#include <thread>
#include <atomic>
#include "sha3.hpp"

class alignas(64) HashPad
{
public:
    inline void init(uint64_t nonce,const Bits<256>& headerHash)
    {
        memcpy(this->smix[0].ptr(), headerHash.ptr(), 32);
        memcpy(this->smix[0].ptr()+32, &nonce, 8);
        *this->smix[3].ptr64(8) = nonce;
    }
    inline uint64_t getNonce() const
    {
        return *this->smix[3].ptr64(8);
    }
    inline const uint8_t* getHashResultPtr()
    {
        return this->smix[3].ptr();
    }
    inline const uint8_t* getMixResultPtr()
    {
        return this->smix[1].ptr();
    }
    inline bool isHashResultLowerThan(const Bits<256>& ref)
    {
        return ref > this->smix[3].ptr();
    }
    inline uint32_t fnv(uint32_t a, uint32_t b)
    {
        //return (a + (a<<1) + (a<<4) + (a<<7) + (a<<8) + (a<<24)) ^ b;
        return (a*0x01000193) ^ b;
    }
    inline void hash(const std::shared_ptr<WorkGraph> workgraph)
    {
        //SHA3_512(this->smix[0].ptr(), this->smix[0].ptr(), 40);
        SHA3_40B_512(this->smix[0].ptr(), this->smix[0].ptr());
        for(int i=0 ; i<8 ; i++)
        {
            *this->smix[1].ptr64(i) = *this->smix[0].ptr64(i);
            *this->smix[2].ptr64(i) = *this->smix[0].ptr64(i);
        }
        
        const uint32_t numPages = (uint32_t)(workgraph->getDAGByteLength() / 128);
        const Bytes<64>* dag = workgraph->getDAG();
        uint32_t* mix = (uint32_t*)smix[1].ptr();
        for (uint32_t i = 0; i != 64; ++i)
        {
            uint32_t index = fnv(smix[0].value32(0)^i, mix[i % 32]) % numPages;
            
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
        
        //SHA3_256(smix[3].ptr(), smix[0].ptr(), 64 + 32);
        SHA3_96B_256(smix[3].ptr(), smix[0].ptr());
    }
    Bytes<64> smix[4];
};

class CpuWorkerFactory : public WorkerFactory
{
public:
    virtual std::unique_ptr<Worker> create(std::string name) override;
};

class alignas(64)  CpuWorker : public Worker
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
    int threadCount;
    std::vector<std::thread> workerThreads;
    std::mutex setWorkMutex;
    size_t numPages;
};

#endif
