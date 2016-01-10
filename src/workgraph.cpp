#include "workgraph.hpp"
#include "sha3.hpp"
#include <iostream>
#include <fstream>
#include "sizetable.hpp"

std::map<uint64_t,std::shared_ptr<WorkGraph>> WorkGraph::cachedWorkGraph;

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

std::shared_ptr<WorkGraph> WorkGraph::getWorkGraph(uint64_t blockNumber)
{
    auto cached = WorkGraph::cachedWorkGraph.find(blockNumber);
    if(cached == WorkGraph::cachedWorkGraph.end())
    {
        std::shared_ptr<WorkGraph> workgraph = std::make_shared<WorkGraph>(blockNumber);
        WorkGraph::cachedWorkGraph[blockNumber] = workgraph;
        return workgraph;
    }
    else
    {
        return cached->second;
    }
}

void WorkGraph::deleteWorkGraph(uint64_t blockNumber)
{
    WorkGraph::cachedWorkGraph.erase(blockNumber);
}

void WorkGraph::deleteWorkGraph(uint64_t blockNumber,std::string dagFile, std::string dagCacheFile)
{
    WorkGraph::cachedWorkGraph.erase(blockNumber);
    remove(dagFile.c_str());
    remove(dagCacheFile.c_str());
}

std::shared_ptr<WorkGraph> WorkGraph::getWorkGraph(uint64_t blockNumber, std::string dagFile, std::string dagCacheFile)
{
    auto cached = WorkGraph::cachedWorkGraph.find(blockNumber);
    if(cached == WorkGraph::cachedWorkGraph.end())
    {
        std::shared_ptr<WorkGraph> workgraph = std::make_shared<WorkGraph>(blockNumber,dagFile,dagCacheFile);
        WorkGraph::cachedWorkGraph[blockNumber] = workgraph;
        return workgraph;
    }
    else
    {
        return cached->second;
    }
}

WorkGraph::WorkGraph(uint64_t blockNumber,std::string dagFile,std::string dagCacheFile)
: Block(blockNumber)
{
    if(!this->readDAGCacheFromFile(dagCacheFile))
    {
        this->computeCache();
        this->writeDAGCacheToFile(dagCacheFile);
    }
    if(!this->readDAGFromFile(dagFile))
    {
        this->computeDAG();
        this->writeDAGToFile(dagFile);
    }
}

WorkGraph::WorkGraph(const WorkGraph& ref)
: Block((const Block&)ref)
{
    if(!this->readDAGCacheFromFile("DAG-Light-"+this->seedHash.toString()))
    {
        this->computeCache();
        this->writeDAGCacheToFile("DAG-Light-"+this->seedHash.toString());
    }
    if(!this->readDAGFromFile("DAG-"+this->seedHash.toString()))
    {
        this->computeDAG();
        this->writeDAGToFile("DAG-"+this->seedHash.toString());
    }
};

WorkGraph::WorkGraph(const WorkGraph& ref, uint64_t blockNumber)
: Block((const Block&)ref,blockNumber)
{
    if(!this->readDAGCacheFromFile("DAG-Light-"+this->seedHash.toString()))
    {
        this->computeCache();
        this->writeDAGCacheToFile("DAG-Light-"+this->seedHash.toString());
    }
    if(!this->readDAGFromFile("DAG-"+this->seedHash.toString()))
    {
        this->computeDAG();
        this->writeDAGToFile("DAG-"+this->seedHash.toString());
    }
};

WorkGraph::WorkGraph(uint64_t blockNumber) : Block(blockNumber)
{
    if(!this->readDAGCacheFromFile("DAG-Light-"+this->seedHash.toString()))
    {
        this->computeCache();
        this->writeDAGCacheToFile("DAG-Light-"+this->seedHash.toString());
    }
    if(!this->readDAGFromFile("DAG-"+this->seedHash.toString()))
    {
        this->computeDAG();
        this->writeDAGToFile("DAG-"+this->seedHash.toString());
    }
};

WorkGraph::WorkGraph(uint64_t blockNumber,Bits<256> seedHash)
: Block(blockNumber, seedHash)
{
    if(!this->readDAGCacheFromFile("DAG-Light-"+this->seedHash.toString()))
    {
        this->computeCache();
        this->writeDAGCacheToFile("DAG-Light-"+this->seedHash.toString());
    }
    if(!this->readDAGFromFile("DAG-"+this->seedHash.toString()))
    {
        this->computeDAG();
        this->writeDAGToFile("DAG-"+this->seedHash.toString());
    }
};
