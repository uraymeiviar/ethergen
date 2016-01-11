#ifndef block_h
#define block_h

#include <stdint.h>
#include "bits.hpp"
#include <map>

typedef uint32_t Epoch;

class Block
{
public:
    static const uint32_t EpochLength = 30000;
    static const uint32_t CacheRound = 3;
    
    const Bits<256> getSeedHash() const;
    uint64_t getBlockNumber() const;
    Epoch getEpoch() const;
    
    Block();
    Block(const Block& ref);
    Block(const Block& ref, uint64_t blockNumber);
    Block(uint64_t blockNumber);
    Block(uint64_t blockNumber,Bits<256> seedHash);
    
    static Epoch blockNumToEpoch(uint64_t blockNumber);
    static Bits<256> getSeedHash(Epoch epoch);
    
    Bits<256> seedHash;
    uint64_t blockNumber;
protected:
    static void createSeedMapCache();
    static std::map<uint64_t,Bits<256>> epochToSeedMap;
    static std::map<Bits<256>,uint64_t> seedToEpochMap;
};

#endif /* block_h */
