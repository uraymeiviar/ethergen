#ifndef block_h
#define block_h

#include <stdint.h>
#include "bits.hpp"

class Block
{
public:
    static const uint32_t EpochLength = 30000;
    static const uint32_t CacheRound = 3;
    
    const Bits<256> getSeedHash() const;
    uint64_t getBlockNumber() const;
    uint64_t getEpoch() const;
    
    Block();
    Block(const Block& ref);
    Block(const Block& ref, uint64_t blockNumber);
    Block(uint64_t blockNumber);
    Block(uint64_t blockNumber,Bits<256> seedHash);
    
    static uint64_t blockNumToEpoch(uint64_t blockNumber);
    static Bits<256> createSeedHash(uint64_t blockNumber);
    static Bits<256> createSeedHash(uint64_t blockNumber, uint64_t prevBlockNumber, const Bits<256>& prevSeed);
protected:
    Bits<256> seedHash;
    uint64_t blockNumber;
};

#endif /* block_h */
