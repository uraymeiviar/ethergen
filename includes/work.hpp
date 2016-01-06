#ifndef Work_h
#define Work_h

#include <vector>
#include <functional>
#include <memory>
#include <map>

#include "bits.hpp"
#include "block.hpp"

class WorkResult
{
public:
    uint64_t nonce;
    uint64_t blockNumber;
    Bits<256> hash;
    Bits<256> mixHash;
    Bits<256> target;
    Bits<256> headerHash;
    Bits<256> seedHash;
    WorkResult()
    {
        this->nonce = 0;
        this->blockNumber = 0;
    };
};

class WorkGraph : public Block
{
public:
    static const uint32_t DatasetParent = 256;
    static const uint32_t FnvPrime = 0x01000193;
    void writeDAGToFile(std::string path);
    void writeDAGCacheToFile(std::string path);
    bool readDAGFromFile(std::string path);
    bool readDAGCacheFromFile(std::string path);
    const Bytes<64>* getDAG() const;
    const Bytes<64>* getDAGCache() const;
    size_t getDAGByteLength() const;
    size_t getDAGCacheByteLength() const;
    
    WorkGraph() : Block() {};
    WorkGraph(const WorkGraph& ref) : Block((const Block&)ref){};
    WorkGraph(const WorkGraph& ref, uint64_t blockNumber) : Block((const Block&)ref,blockNumber){};
    WorkGraph(uint64_t blockNumber) : Block(blockNumber) {};
    WorkGraph(uint64_t blockNumber,std::string dagFile,std::string dagCacheFile);
    WorkGraph(uint64_t blockNumber,Bits<256> seedHash) : Block(blockNumber, seedHash){};
    
    static uint32_t getCacheSize(uint64_t blockNumber);
    static uint64_t getDAGSize(uint64_t blockNumber);
    static uint32_t fnvHash(uint32_t x, uint32_t y);
    static void deleteWorkGraph(uint64_t blockNumber);
    static void deleteWorkGraph(uint64_t blockNumber,std::string dagFile, std::string dagCacheFile);
    static std::shared_ptr<WorkGraph> getWorkGraph(uint64_t blockNumber);
    static std::shared_ptr<WorkGraph> getWorkGraph(uint64_t blockNumber, std::string dagFile, std::string dagCacheFile);
protected:
    std::vector<Bytes<64>> cache;
    std::vector<Bytes<64>> dag;
    void computeCache();
    void computeDAGItem(Bytes<64>& target, size_t index);
    void computeDAGItem(Bytes<64>& target, size_t index, const std::vector<Bytes<64>>& cache);
    void computeDAG();
    static void registerWorkGraph(std::shared_ptr<WorkGraph>, uint64_t blockNumber);
    static std::map<uint64_t,std::shared_ptr<WorkGraph>> cachedWorkGraph;
};

class Work : public Block
{
public:
    Bits<256> target;
    Bits<256> headerHash;
    static const uint32_t MixBytes = 128;
    static const uint32_t MixDWord = 32;
    static const uint32_t MixNodes = 2;
    
    Work() : Block() {};
    Work(const Work& ref) : Block((const Block&)ref){};
    Work(const Work& ref, uint64_t blockNumber) : Block((const Block&)ref,blockNumber){};
    Work(uint64_t blockNumber) : Block(blockNumber) {};
    Work(uint64_t blockNumber,Bits<256> seedHash) : Block(blockNumber, seedHash){};
    bool checkNonce(uint64_t nonce, WorkResult& result, std::shared_ptr<const WorkGraph> graph);
protected:
};

#endif /* Work_h */
