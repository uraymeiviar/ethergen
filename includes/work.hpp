#ifndef Work_h
#define Work_h

#include <vector>
#include <functional>
#include <memory>

#include "bits.hpp"
#include "block.hpp"
#include "workgraph.hpp"

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
