#include "block.hpp"
#include "sha3.hpp"

Bits<256> Block::createSeedHash(uint64_t blockNumber)
{
    uint64_t const epochs = blockNumber / Block::EpochLength;
    Bits<256> seedHash;
    for (uint64_t i = 0; i < epochs; ++i)
    {
        SHA3_256(seedHash.ptr(), seedHash.ptr(), 32);
    }
    return seedHash;
}

Bits<256> Block::createSeedHash(uint64_t blockNumber, uint64_t prevBlockNumber, const Bits<256>& prevSeed)
{
    uint64_t const epochs = blockNumber / Block::EpochLength;
    uint64_t const prevEpoch = prevBlockNumber / Block::EpochLength;
    uint64_t const remainingEpoch = epochs - prevEpoch;
    Bits<256> seedHash(prevSeed);
    for (uint64_t i = 0; i < remainingEpoch; ++i)
    {
        SHA3_256(seedHash.ptr(), seedHash.ptr(), 32);
    }
    return seedHash;
}

const Bits<256> Block::getSeedHash() const
{
    return this->seedHash;
}

uint64_t Block::getBlockNumber() const
{
    return this->blockNumber;
}

uint64_t Block::getEpoch() const
{
    return this->blockNumber / Block::EpochLength;
}

uint64_t Block::blockNumToEpoch(uint64_t blockNumber)
{
    return blockNumber / Block::EpochLength;
}

Block::Block()
{
    this->blockNumber = 0;
    this->seedHash = Block::createSeedHash(0);
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
    else if(this->getEpoch() > ref.getEpoch())
    {
        this->seedHash = Block::createSeedHash(blockNumber,ref.getBlockNumber(),ref.getSeedHash());
    }
    else
    {
        this->seedHash = Block::createSeedHash(blockNumber);
    }
}

Block::Block(uint64_t blockNumber)
{
    this->blockNumber = blockNumber;
    this->seedHash = Block::createSeedHash(blockNumber);
}

Block::Block(uint64_t blockNumber,Bits<256> seedHash)
{
    this->blockNumber = blockNumber;
    this->seedHash = seedHash;
}