#include <iostream>
#include <string>
#include <algorithm>
#include "work.hpp"

std::string version = "0.1";

void printUsage(const char* cmd)
{
    std::cout << "ETHERGEN " << version.c_str() << std::endl;
    std::cout << "usage  " << cmd << " <mode>" << std::endl;
    std::cout << "mode               | parameters " << std::endl;
    std::cout << "     calc-dag-node   <block-number>" << std::endl;
    std::cout << "     calc-dag        <block-number>" << std::endl;
    std::cout << "     hash            <block-number> <nonce> <headerhash> [target]" << std::endl;
    std::cout << "     benchmark       <cpu|gpu> <threads>" << std::endl;
    std::cout << "     gen             <cpu|gpu> <threads>" << std::endl;
    std::cout << "     help" << std::endl;
    std::cout << std::endl;
}

void calcDagNode(int argc, const char * argv[])
{
    if(argc < 3)
    {
        printUsage(argv[0]);
    }
    else
    {
        uint64_t blockNumber = strtoull(argv[2],nullptr,10);
        Block block(blockNumber);
        std::cout << "Block Number " << std::to_string(blockNumber) << std::endl;
        std::cout << "       Epoch " << std::to_string(block.getEpoch()) << std::endl;
        std::cout << "   Seed Hash " << block.getSeedHash().toString() << std::endl;
        std::cout << "   Node Size " << std::to_string(WorkGraph::getCacheSize(blockNumber)) << std::endl;
        std::cout << "    DAG Size " << std::to_string(WorkGraph::getDAGSize(blockNumber)) << std::endl;
    }
}

void calcDag(int argc, const char* argv[])
{
    if(argc < 3)
    {
        printUsage(argv[0]);
    }
    else
    {
        uint64_t blockNumber = strtoull(argv[2],nullptr,10);
        uint64_t epoch = Block::blockNumToEpoch(blockNumber);
        Bits<256> seedHash = Block::createSeedHash(blockNumber);
        
        std::string lightPath = "DAG-Light-"+std::to_string(epoch)+"-"+seedHash.toString();
        std::string fullPath = "DAG-"+std::to_string(epoch)+"-"+seedHash.toString();
        std::shared_ptr<WorkGraph> workGraph = WorkGraph::getWorkGraph(blockNumber,fullPath,lightPath);
    }
}

void hash(int argc, const char* argv[])
{
    if(argc < 5)
    {
        printUsage(argv[0]);
    }
    else
    {
        uint64_t blockNumber = strtoull(argv[2],nullptr,10);
        uint64_t epoch = Block::blockNumToEpoch(blockNumber);
        uint64_t nonce = strtoull(argv[3],nullptr,10);
        Bits<256> seedHash = Block::createSeedHash(blockNumber);
        std::string headerHash = std::string(argv[4]);
        std::string target = "0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
        
        if(argc > 5)
        {
            target = std::string(argv[5]);
        }
        
        std::string lightPath = "DAG-Light-"+std::to_string(epoch)+"-"+seedHash.toString();
        std::string fullPath = "DAG-"+std::to_string(epoch)+"-"+seedHash.toString();
        std::shared_ptr<WorkGraph> workGraph = WorkGraph::getWorkGraph(blockNumber, fullPath, lightPath);
        
        Work work(blockNumber);
        work.headerHash.fromString(headerHash);
        work.target.fromString(target);
        
        WorkResult result;
        bool targetResult = work.checkNonce(nonce,result,workGraph);
        
        std::cout << "Block Number " << std::to_string(blockNumber) << std::endl;
        std::cout << "       Epoch " << std::to_string(workGraph->getEpoch()) << std::endl;
        std::cout << "   Seed Hash " << workGraph->getSeedHash().toString() << std::endl;
        std::cout << " Header Hash " << work.headerHash.toString() << std::endl;
        std::cout << "   Node Size " << std::to_string(workGraph->getDAGCacheByteLength()) << std::endl;
        std::cout << "    DAG Size " << std::to_string(workGraph->getDAGByteLength()) << std::endl;
        std::cout << "    Mix Hash " << result.mixHash.toString() << std::endl;
        std::cout << "       Nonce " << std::to_string(result.nonce) << std::endl;
        std::cout << " Target Hash " << work.target.toString() << std::endl;
        std::cout << " Hash Result " << result.hash.toString() << std::endl;
        std::cout << "Below Target " << std::to_string(targetResult) << std::endl;
    }
}

int main(int argc, const char * argv[])
{
    if(argc < 2)
    {
        printUsage(argv[0]);
    }
    else
    {
        std::string mode = std::string(argv[1]);
        
        if(mode == "help")
        {
            printUsage(argv[0]);
        }
        else if(mode == "calc-dag-node")
        {
            calcDagNode(argc,argv);
        }
        else if(mode == "calc-dag")
        {
            calcDag(argc,argv);
        }
        else if(mode == "hash")
        {
            hash(argc,argv);
        }
        else
        {
            std::cout << "unknown execution mode" << std::endl;
            printUsage(argv[0]);
        }
    }
    return 0;
}