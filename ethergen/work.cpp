#include "work.hpp"
#include "sha3.hpp"
#include "sizetable.hpp"
#include <fstream>
#include <iostream>

void WorkGraph::computeCache()
{
    size_t nodeCount = WorkGraph::getCacheSize(this->blockNumber)/64;
    this->cache.resize(nodeCount);
    for(size_t i=0 ; i<this->cache.size() ; i++)
    {
        this->cache[i].reset();
    }
    SHA3_512(this->cache[0].ptr(), this->seedHash.ptr(), 32);
    
    for (size_t i = 1; i != nodeCount; ++i)
    {
        SHA3_512(this->cache[i].ptr(), this->cache[i - 1].ptr(), 64);
    }
    
    for (size_t j = 0; j != Block::CacheRound; j++)
    {
        for (size_t i = 0; i != nodeCount; i++)
        {
            size_t const idx = this->cache[i].value32(0) % nodeCount;
            Bytes<64> data = this->cache[(nodeCount - 1 + i) % nodeCount];
            for (size_t w = 0; w != Bytes<64>::byteSize()/4; ++w) {
                data.value32(w, data.value32(w) ^ this->cache[idx].value32(w));
            }
            SHA3_512(this->cache[i].ptr(), data.ptr(), data.byteSize());
        }
    }
}

void WorkGraph::computeDAGItem(Bytes<64>& target, size_t index)
{
    if(this->cache.size()*64 != WorkGraph::getCacheSize(this->blockNumber))
    {
        this->computeCache();
    }
    this->computeDAGItem(target,index,this->cache);
}

void WorkGraph::computeDAGItem(Bytes<64>& result, size_t index, const std::vector<Bytes<64>>& cache)
{
    uint32_t numParentNode = (uint32_t)cache.size();
    uint32_t nodeNdx = (uint32_t)index;
    result = cache[index % numParentNode];
    result.value32(0,result.value32(0) ^ nodeNdx);
    SHA3_512(result.ptr(), result.ptr(), 64);
    
    for(uint32_t i=0 ; i<WorkGraph::DatasetParent ; i++)
    {
        uint32_t parentNdx = WorkGraph::fnvHash(nodeNdx ^ i, result.value32(i % 16)) % numParentNode;
        for(size_t w=0 ; w<16 ; w++)
        {
            result.value32(w,WorkGraph::fnvHash( result.value32(w) , cache[parentNdx].value32(w)));
        }
    }
    
    SHA3_512(result.ptr(), result.ptr(), 64);
}

void WorkGraph::computeDAG()
{
    uint64_t dagBytesSize = WorkGraph::getDAGSize(this->blockNumber);
    this->dag.resize(dagBytesSize/64);
    for(size_t n = 0 ; n<this->dag.size(); n++)
    {
        this->computeDAGItem(this->dag[n], n);
        if(n % 1000 == 0)
        {
            size_t progress = 100*n / this->dag.size();
            std::cout << "creating DAG node #" <<
                std::to_string(n) << " of " << this->dag.size() <<
                " ( " << std::to_string(progress)<< "% ) \r";
            std::cout.flush();
        }
    }
}

void WorkGraph::writeDAGToFile(std::string path)
{
    std::ofstream outputFile( path, std::ofstream::out );
    if(this->dag.size()*64 != WorkGraph::getDAGSize(this->blockNumber))
    {
        this->computeDAG();
    }
    for(size_t i= 0 ; i<this->dag.size() ; i++)
    {
        outputFile.write((const char*)this->dag[i].ptr(), this->dag[i].byteSize());
    }
    outputFile.flush();
    outputFile.close();
}

void WorkGraph::writeDAGCacheToFile(std::string path)
{
    std::ofstream outputFile( path, std::ofstream::out );
    if(this->cache.size()*64 != WorkGraph::getCacheSize(this->blockNumber))
    {
        this->computeCache();
    }
    for(size_t i= 0 ; i<this->cache.size() ; i++)
    {
        outputFile.write((const char*)this->cache[i].ptr(), this->cache[i].byteSize());
    }
    outputFile.flush();
    outputFile.close();
}

bool WorkGraph::readDAGCacheFromFile(std::string path)
{
    std::ifstream inputFile( path, std::ios::binary );
    if(inputFile.fail())
    {
        return false;
    }
    else
    {
        size_t nodeCount = WorkGraph::getCacheSize(this->blockNumber)/64;
        this->cache.resize(nodeCount);
        for(size_t i=0 ; i<this->cache.size() ; i++)
        {
            inputFile.read((char*)this->cache[i].ptr(), this->cache[i].byteSize());
        }
        inputFile.close();
        return true;
    }
}

bool WorkGraph::readDAGFromFile(std::string path)
{
    std::ifstream inputFile( path, std::ios::binary );
    if(inputFile.fail())
    {
        return false;
    }
    else
    {
        uint64_t dagBytesSize = WorkGraph::getDAGSize(this->blockNumber);
        this->dag.resize(dagBytesSize/64);
        for(size_t i=0 ; i<this->dag.size() ; i++)
        {
            inputFile.read((char*)this->dag[i].ptr(), this->dag[i].byteSize());
        }
        inputFile.close();
        return true;
    }
}

const Bytes<64>* WorkGraph::getDAG() const
{
    if(this->dag.size()*64 != WorkGraph::getDAGSize(this->blockNumber))
    {
        return nullptr;
    }
    return this->dag.data();
}

const Bytes<64>* WorkGraph::getDAGCache() const
{
    if(this->cache.size()*64 != WorkGraph::getCacheSize(this->blockNumber))
    {
        return nullptr;
    }
    return this->dag.data();
}

size_t WorkGraph::getDAGByteLength() const
{
    return this->dag.size()*64;
}

size_t WorkGraph::getDAGCacheByteLength() const
{
    return this->cache.size()*64;
}

uint32_t WorkGraph::fnvHash(uint32_t x, uint32_t y)
{
    return x * WorkGraph::FnvPrime ^ y;
}

uint32_t WorkGraph::getCacheSize(uint64_t blockNumber)
{
    return cache_sizes[blockNumber / Block::EpochLength];
}

uint64_t WorkGraph::getDAGSize(uint64_t blockNumber)
{
    return dag_sizes[blockNumber / Block::EpochLength];
}

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

bool Work::checkNonce(uint64_t nonce, WorkResult& result, const WorkGraph& graph)
{
    std::array<Bytes<64>,3> smix;
    memcpy(smix[0].ptr(), this->headerHash.ptr(), 32);
    memcpy((char*)smix[0].ptr64(4), &nonce, 8);
    SHA3_512(smix[0].ptr(), smix[0].ptr(), 40);
    
    uint32_t* mix = (uint32_t*)smix[1].ptr();
    for (uint32_t w = 0; w != 2*16; ++w)
    {
        mix[w] = smix[0].value32(w % 16);
    }
    
    size_t numPages = graph.getDAGByteLength() / 128;
    
    for (uint32_t i = 0; i != 64; ++i)
    {
        uint32_t index = WorkGraph::fnvHash(smix[0].value32(0) ^ i, mix[i % 32]) % numPages;
        
        for (size_t n = 0; n != 2; ++n)
        {
            const Bytes<64>* dagNode = &graph.getDAG()[2 * index + n];
            
            for (size_t w = 0; w != 16; ++w)
            {
                mix[n*16 + w] = WorkGraph::fnvHash(mix[n*16 + w], dagNode->value32(w));
            }
        }
    }
    
    for (size_t w = 0; w != 32; w += 4)
    {
        uint32_t reduction = mix[w + 0];
        reduction = reduction * WorkGraph::FnvPrime ^ mix[w + 1];
        reduction = reduction * WorkGraph::FnvPrime ^ mix[w + 2];
        reduction = reduction * WorkGraph::FnvPrime ^ mix[w + 3];
        mix[w / 4] = reduction;
    }
    
    memcpy(result.mixHash.ptr(), mix, 32);
    SHA3_256(result.hash.ptr(), smix[0].ptr(), 64 + 32); // Keccak-256(s + compressed_mix)
    result.target = this->target;
    result.headerHash = this->headerHash;
    result.seedHash = this->getSeedHash();
    result.nonce = nonce;
    result.blockNumber = this->getBlockNumber();
    
    return result.hash < result.target;
}