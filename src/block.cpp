#include "block.hpp"
#include "sha3.hpp"
#include <iostream>
#include <fstream>

std::map<uint64_t,Bits<256>> Block::epochToSeedMap;
std::map<Bits<256>,uint64_t> Block::seedToEpochMap;

Bits<256> Block::getSeedHash(Epoch epochs)
{
    if(Block::epochToSeedMap.size() <= 0 || Block::seedToEpochMap.size() <= 0)
    {
        createSeedMapCache();
    }
    if(epochs < 2048)
    {
        return Block::epochToSeedMap[epochs];
    }
    else
    {
        Bits<256> seedHash = Block::epochToSeedMap[2047];
        for (uint64_t i = 2047; i < epochs; ++i)
        {
            SHA3_256(seedHash.ptr(), seedHash.ptr(), 32);
        }
        return seedHash;
    }
}

void Block::createSeedMapCache()
{
    uint64_t seedMapFileSize = 2048*(256/8);
    uint8_t seedMap[seedMapFileSize];
    
    std::ifstream inputFile( "seedmap", std::ios::binary );
    if(inputFile.fail())
    {
        Bits<256> seedHash;
        for(size_t i=0 ; i<2048 ; i++)
        {
            uint8_t* ptr = &seedMap[i*(256/8)];
            memcpy(ptr,seedHash.ptr(),32);
            
            Block::epochToSeedMap[i] = Bits<256>(seedHash);
            Block::seedToEpochMap[seedHash] = i;
            
            SHA3_256(seedHash.ptr(), seedHash.ptr(), 32);
        }
        
        std::ofstream outputFile( "seedmap", std::ofstream::out );
        outputFile.write((char*)seedMap, seedMapFileSize);
        outputFile.flush();
        outputFile.close();
    }
    else
    {
        inputFile.read((char*)seedMap,seedMapFileSize);
        inputFile.close();
        for(size_t i=0 ; i<2048 ; i++)
        {
            uint8_t* ptr = &seedMap[i*(256/8)];
            Block::epochToSeedMap[i] = Bits<256>(ptr);
            Block::seedToEpochMap[Bits<256>(ptr)] = i;
        }
    }
}

const Bits<256> Block::getSeedHash() const
{
    return this->seedHash;
}

uint64_t Block::getBlockNumber() const
{
    return this->blockNumber;
}

Epoch Block::getEpoch() const
{
    return (Epoch)(this->blockNumber / Block::EpochLength);
}

Epoch Block::blockNumToEpoch(uint64_t blockNumber)
{
    return (Epoch)(blockNumber / Block::EpochLength);
}

Block::Block()
{
    this->blockNumber = 0;
    this->seedHash = Block::getSeedHash((Epoch)0);
}

Block::Block(const Block& ref)
{
    this->blockNumber = ref.blockNumber;
    this->seedHash = ref.seedHash;
}

Block::Block(const Block& ref,uint64_t blockNumber)
{
    this->blockNumber = blockNumber;
    if(ref.getEpoch() == this->getEpoch())
    {
        this->seedHash = ref.seedHash;
    }
    this->seedHash = Block::getSeedHash(this->getEpoch());
}

Block::Block(uint64_t blockNumber)
{
    this->blockNumber = blockNumber;
    this->seedHash = Block::getSeedHash(this->getEpoch());
}

Block::Block(uint64_t blockNumber,Bits<256> seedHash)
{
    this->blockNumber = blockNumber;
    this->seedHash = seedHash;
}