//
//  Work.h
//  ethergen
//
//  Created by Uray Meiviar on 1/3/16.
//  Copyright © 2016 etherlink. All rights reserved.
//

#ifndef Work_h
#define Work_h
#include "bits.hpp"
#include <vector>

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
    
    static Bits<256> createSeedHash(uint64_t blockNumber);
    static Bits<256> createSeedHash(uint64_t blockNumber, uint64_t prevBlockNumber, const Bits<256>& prevSeed);
protected:
    Bits<256> seedHash;
    uint64_t blockNumber;
};

class WorkGraph : public Block
{
public:
    static const uint32_t DatasetParent = 256;
    static const uint32_t FnvPrime = 0x01000193;
    WorkGraph() : Block() {};
    WorkGraph(const WorkGraph& ref) : Block((const Block&)ref){};
    WorkGraph(const WorkGraph& ref, uint64_t blockNumber) : Block((const Block&)ref,blockNumber){};
    WorkGraph(uint64_t blockNumber) : Block(blockNumber) {};
    WorkGraph(uint64_t blockNumber,Bits<256> seedHash) : Block(blockNumber, seedHash){};
    void writeDAGToFile(std::string path);
    void writeDAGCacheToFile(std::string path);
    bool readDAGFromFile(std::string path);
    bool readDAGCacheFromFile(std::string path);
    const Bytes<64>* getDAG() const;
    const Bytes<64>* getDAGCache() const;
    size_t getDAGByteLength() const;
    size_t getDAGCacheByteLength() const;
    
    static uint32_t getCacheSize(uint64_t blockNumber);
    static uint64_t getDAGSize(uint64_t blockNumber);
    static uint32_t fnvHash(uint32_t x, uint32_t y);
protected:
    std::vector<Bytes<64>> cache;
    std::vector<Bytes<64>> dag;
    void computeCache();
    void computeDAGItem(Bytes<64>& target, size_t index);
    void computeDAGItem(Bytes<64>& target, size_t index, const std::vector<Bytes<64>>& cache);
    void computeDAG();
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
    bool checkNonce(uint64_t nonce, WorkResult& result, const WorkGraph& graph);
protected:
};

#endif /* Work_h */
