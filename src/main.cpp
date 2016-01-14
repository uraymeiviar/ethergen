#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <streambuf>
#include "work.hpp"
#include "miner.hpp"
#include "cpuworker.hpp"
#include "gpuworker.h"
#include "poolconnection.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/error/error.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "cl/cl.h"

std::string version = "0.1";

std::map<std::string,WorkerFactory*> workerFactoryMap;
std::map<std::string,ConnectionFactory*> connectionFactoryMap;
void initFactories()
{
    workerFactoryMap["cpu"] = &CpuWorker::getFactory();
    connectionFactoryMap["getwork"] = &PoolConnection::getFactory();
}

void printUsage(const char* cmd)
{
    std::cout << "ETHERGEN " << version.c_str() << std::endl;
    std::cout << "usage  " << cmd << " <mode>" << std::endl;
    std::cout << "mode               | parameters " << std::endl;
    std::cout << "     calc-dag-node   <block-number>" << std::endl;
    std::cout << "     calc-dag        <block-number>" << std::endl;
    std::cout << "     hash            <block-number> <nonce> <headerhash> [target]" << std::endl;
    std::cout << "     benchmark       <config-file>" << std::endl;
    std::cout << "     clinfo          <cpu|gpu|all>" << std::endl;
    std::cout << "     gen             <config-file>" << std::endl;
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
        std::cout << "   Node Size " << std::to_string(WorkGraph::getDAGCacheSize(block.getEpoch())) << std::endl;
        std::cout << "    DAG Size " << std::to_string(WorkGraph::getDAGSize(block.getEpoch())) << std::endl;
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
        Epoch epoch = Block::blockNumToEpoch(blockNumber);
        Bits<256> seedHash = Block::getSeedHash(epoch);
        
        std::string lightPath = "DAG-Light-"+std::to_string(epoch)+"-"+seedHash.toString();
        std::string fullPath = "DAG-"+std::to_string(epoch)+"-"+seedHash.toString();
        std::shared_ptr<WorkGraph> workGraph = WorkGraph::getWorkGraph(epoch,fullPath,lightPath);
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
        Epoch epoch = Block::blockNumToEpoch(blockNumber);
        uint64_t nonce = strtoull(argv[3],nullptr,10);
        Bits<256> seedHash = Block::getSeedHash(epoch);
        std::string headerHash = std::string(argv[4]);
        std::string target = "0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
        
        if(argc > 5)
        {
            target = std::string(argv[5]);
        }
        
        std::string lightPath = "DAG-Light-"+seedHash.toString();
        std::string fullPath = "DAG-"+seedHash.toString();
        std::shared_ptr<WorkGraph> workGraph = WorkGraph::getWorkGraph(epoch, fullPath, lightPath);
        
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

void openclInfo(int argc, const char* argv[])
{
    cl_device_type clType = CL_DEVICE_TYPE_ALL;
    if(argc > 2)
    {
        std::string typeStr = std::string(argv[2]);
        if(typeStr == "cpu" || typeStr == "CPU")
        {
            clType = CL_DEVICE_TYPE_CPU;
        }
        else if(typeStr == "gpu" || typeStr == "GPU")
        {
            clType = CL_DEVICE_TYPE_GPU;
        }
    }
    std::map<cl_platform_id,std::vector<cl_device_id>> devices;
    GpuWorker::getAllDevices(devices,clType);
    ClDeviceInfo deviceInfo;
    
    for(auto &x : devices){
        deviceInfo.platform = x.first;
        for(auto &d : x.second)
        {
            deviceInfo.device = d;
            if(ClDeviceInfo::getInfo(deviceInfo))
            {
                std::string endian = "Endian:L";
                if(!deviceInfo.endianLittle)
                {
                    endian = "Endian:B";
                }
                std::string wiSize = std::to_string(deviceInfo.maxWorkItemSize[0]);
                for(size_t w = 1 ; w<deviceInfo.maxWorkItemSize.size() ; w++)
                {
                    wiSize += "x"+std::to_string(deviceInfo.maxWorkItemSize[w]);
                }
                std::cout << deviceInfo.platform << ":" << deviceInfo.device << " " << deviceInfo.name << " ( " << ClDeviceInfo::deviceTypeToString(deviceInfo.type) << ") " << deviceInfo.version << "/ SW." << deviceInfo.driverVersion << " " << deviceInfo.vendor << " (0x" << std::hex << deviceInfo.vendorId << ") " << endian << std::endl;
                
                std::cout << "  GMemSize: " << std::to_string(deviceInfo.globalMemorySize/(1024*1024)) << "MB" <<
                "  GCacheSize: " << std::to_string(deviceInfo.globalMemoryCacheSize) <<
                "  GCacheLine: " << std::to_string(deviceInfo.globalMemoryCacheLineSize/1024) << "KB" <<
                "  LMemSize: " << std::to_string(deviceInfo.localMemorySize/1024) << "KB" << std::endl;
                
                std::cout << "  MaxClock: " << std::to_string(deviceInfo.maxClock) << "MHz" <<
                "  MaxCU: " << std::to_string(deviceInfo.maxComputeUnit) <<
                "  MaxConst: " << std::to_string(deviceInfo.maxConstant) << "x" << std::to_string(deviceInfo.maxConstantBufferSize/1024) << "KB" <<
                "  MaxAlloc: " << std::to_string(deviceInfo.maxMemoryAllocSize/(1024*1024)) << "MB" << std::endl;
                std::cout << "  MaxParamSize: " << std::to_string(deviceInfo.maxParameterSize) <<
                "  MaxWGSize: " << std::to_string(deviceInfo.maxWorkGroupSize) <<
                "  MaxWISize: " << wiSize << std::endl << std::endl;
            }
        }
    }
}

void gen(int argc, const char* argv[])
{
    if(argc < 3)
    {
        printUsage(argv[0]);
    }
    else
    {
        std::string configFile = std::string(argv[2]);
        std::ifstream inputFile( configFile, std::ios::binary );
        if(inputFile.fail())
        {
            std::cout << "unable to open file " << configFile << std::endl;
        }
        else
        {
            std::string configStr((std::istreambuf_iterator<char>(inputFile)),
                                  std::istreambuf_iterator<char>());
            inputFile.close();
            
            rapidjson::Document configJson;
            rapidjson::ParseResult parseResult = configJson.Parse(configStr.c_str());
            
            if (!parseResult) {
                std::cout << "config JSON parse error: " <<
                rapidjson::GetParseError_En(parseResult.Code()) <<
                ":" << std::to_string(parseResult.Offset()) << std::endl;
            }
            else
            {
                if(configJson["workers"].IsArray())
                {
                    Miner miner;
                    for(rapidjson::SizeType i=0 ; i<configJson["workers"].Size() ; i++)
                    {
                        rapidjson::Value& worker = configJson["workers"][i];
                        std::string workerName = "worker";
                        std::string workerType = "cpu";
                        std::string connectionType = "getwork";
                        std::string accountId = "0xdeb215d455c3c065eec3e7cfbe581d01adddb9c6";
                        std::string endpoint = "http://sg.node.etherlink.co/<account>/<worker>";
                        std::string workerParams = "";
                        if(worker.HasMember("name") && worker["name"].IsString())
                        {
                            workerName = worker["name"].GetString();
                        }
                        if(worker.HasMember("type") && worker["type"].IsString())
                        {
                            workerType = worker["type"].GetString();
                        }
                        if(worker.HasMember("params"))
                        {
                            if(worker["params"].IsObject())
                            {
                                rapidjson::StringBuffer buffer;
                                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                                worker["params"].Accept(writer);
                                workerParams = buffer.GetString();
                            }
                            else if(worker["params"].IsString())
                            {
                                workerParams = worker["params"].GetString();
                            }
                        }
                        if(worker.HasMember("connection") && worker["connection"].IsObject())
                        {
                            if(worker["connection"].HasMember("type") && worker["connection"]["type"].IsString())
                            {
                                connectionType = worker["connection"]["type"].GetString();
                            }
                            if(worker["connection"].HasMember("account") && worker["connection"]["account"].IsString())
                            {
                                accountId = worker["connection"]["account"].GetString();
                            }
                            if(worker["connection"].HasMember("endpoint") && worker["connection"]["endpoint"].IsString())
                            {
                                endpoint = worker["connection"]["endpoint"].GetString();
                            }
                        }
                        
                        if(workerFactoryMap.find(workerType) != workerFactoryMap.end())
                        {
                            if(connectionFactoryMap.find(connectionType) != connectionFactoryMap.end())
                            {
                                Bits<160> accountHash(accountId);
                                miner.addWorker(*workerFactoryMap[workerType],
                                                *connectionFactoryMap[connectionType],
                                                workerName,
                                                endpoint,
                                                accountHash,
                                                workerParams);
                            }
                            else
                            {
                                std::cout << "unknown connection type of " << connectionType << std::endl;
                            }
                        }
                        else
                        {
                            std::cout << "unknown worker type of " << workerType << std::endl;
                        }
                    }
                    
                    if(miner.countWorker() > 0)
                    {
                        miner.setRun(true);
                        while(true)
                        {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            uint64_t noncePerSec = miner.getCurrentHashrate();
                            Bits<256> bestResult = miner.getBestResult();
                            std::cout << "best result " << bestResult.toString() << " " << noncePerSec << " nonces/sec" << std::endl;
                            miner.update();
                        }
                    }
                    else
                    {
                        std::cout << "miner does not have worker added" << std::endl;
                    }
                }
                else
                {
                    std::cout << "config JSON does not contain workers list" << std::endl;
                }
            }
        }
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
        else if(mode == "clinfo")
        {
            openclInfo(argc,argv);
        }
        else if(mode == "gen")
        {
            initFactories();
            gen(argc,argv);
        }
        else
        {
            std::cout << "unknown execution mode" << std::endl;
            printUsage(argv[0]);
        }
    }
    return 0;
}
