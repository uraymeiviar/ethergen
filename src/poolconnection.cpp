#include "poolconnection.h"
#include <iostream>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include <cstdlib>
#include <ctime>

template <typename I> std::string hexstr(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789abcdef";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

PoolConnectionFactory& PoolConnection::getFactory()
{
    static PoolConnectionFactory factory;
    return factory;
}

std::unique_ptr<Connection> PoolConnectionFactory::create(const Bits<160>& account, std::string workerName, std::string endpoint)
{
    return std::make_unique<PoolConnection>(account,workerName,endpoint);
}

PoolConnection::PoolConnection(const Bits<160>& account, std::string workerName, std::string endpoint)
{
    std::srand((unsigned)std::time(0));
    this->setAccountId(account);
    this->setWorkerName(workerName);
    this->setEndPoint(endpoint);
    this->httpBodyGetWork = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getWork\",\"params\":[],\"id\":73}";
    this->httpBodyGetBlockNum = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_blockNumber\",\"params\":[],\"id\":83}";
}

std::string PoolConnection::getPoolUrl() const
{
    std::string url = this->endPoint;
    std::string accountToken = "<account>";
    size_t accountTokenPos = url.find(accountToken);
    if(accountTokenPos != std::string::npos)
    {
        url.replace(accountTokenPos,accountToken.length(),this->accountId.toString());
    }
    
    std::string workerToken = "<worker>";
    size_t workerTokenPos = url.find(workerToken);
    if(workerTokenPos != std::string::npos)
    {
        url.replace(workerTokenPos, workerToken.length(), this->workerName);
    }
    return url;
}

uint64_t PoolConnection::getBlockNumber()
{
    std::string httpReply = this->httpClient.httpPost(this->getPoolUrl(), this->httpBodyGetBlockNum);
    if(httpReply.length() > 2)
    {
        rapidjson::Document jsonReply;
        rapidjson::ParseResult parseResult = jsonReply.Parse(httpReply.c_str());
        
        if(parseResult && jsonReply.HasMember("result") && jsonReply["result"].IsString())
        {
            std::string blockNumStr = jsonReply["result"].GetString();
            if(blockNumStr.substr(0,2) == "0x")
            {
                blockNumStr = blockNumStr.substr(2,blockNumStr.length()-2);
            }
            uint64_t blockNum = std::stoull(blockNumStr,nullptr,16);
            return blockNum;
        }
    }
    return -1;
}

void PoolConnection::requestNewWork()
{
    std::string httpReply = this->httpClient.httpPost(this->getPoolUrl(), this->httpBodyGetWork);
    if(httpReply.length() > 2)
    {
        rapidjson::Document jsonReply;
        rapidjson::ParseResult parseResult = jsonReply.Parse(httpReply.c_str());
        
        if(parseResult && jsonReply.HasMember("result") && jsonReply["result"].IsArray())
        {
            rapidjson::Value& result = jsonReply["result"];
            if(result.Size() > 2)
            {
                Bits<256> headerHash(result[0].GetString());
                Bits<256> target(result[2].GetString());
                
                if(this->lastWork.headerHash != headerHash)
                {
                    uint64_t blockNum = this->getBlockNumber();
                    if(blockNum != (uint64_t)-1)
                    {
                        this->lastWork.headerHash = headerHash;
                        this->lastWork.target = target;
                        this->lastWork.seedHash.fromString(result[1].GetString());
                        this->lastWork.blockNumber = blockNum;
                        
                        if(this->onNewWork)
                        {
                            std::cout << "new work block #" << std::to_string(this->lastWork.blockNumber) << std::endl;
                            std::cout << "header hash " << this->lastWork.headerHash.toString() << std::endl;
                            std::cout << "seed hash   " << this->lastWork.seedHash.toString() << std::endl;
                            std::cout << "target      " << this->lastWork.target.toString() << std::endl;
                            std::cout << "-------------------------------" << std::endl;
                            uint64_t startNonce = (uint64_t)this ^ ( ((uint64_t)std::rand()<<32) | (uint64_t)std::rand());
                            this->onNewWork(*this,this->lastWork,startNonce);
                        }
                    }
                }
                else if(this->lastWork.target != target)
                {
                    this->lastWork.target = target;
                    
                    if(this->onNewTarget)
                    {
                        std::cout << "new target  " << this->lastWork.target.toString() << std::endl;
                        this->onNewTarget(*this,this->lastWork.target);
                    }
                }
            }
        }
    }
}

void PoolConnection::submitResult(const WorkResult& result)
{
    std::string httpBody = "{\"jsonrpc\":\"2.0\", \"method\":\"eth_submitWork\", \"params\":[";
    httpBody += "\"0x"+hexstr<uint64_t>(result.nonce)+"\",";
    httpBody += "\""+result.headerHash.toString()+"\",";
    httpBody += "\""+result.mixHash.toString()+"\"";
    httpBody += "],\"id\":73}";
    
    std::string httpReply = this->httpClient.httpPost(this->getPoolUrl(), httpBody);
    if(httpReply.length() > 2)
    {
        rapidjson::Document jsonReply;
        jsonReply.Parse(httpReply.c_str());
        
        if(jsonReply["result"].IsBool())
        {
            if(jsonReply["result"].GetBool())
            {
                std::cout << "-------- work accepted --------" << std::endl;
            }
            else
            {
                std::cout << "-------- work rejected --------" << std::endl;
            }
        }
        
        std::cout << "nonce       0x" << hexstr<uint64_t>(result.nonce) << std::endl;
        std::cout << "header hash " << result.headerHash.toString() << std::endl;
        std::cout << "seed hash   " << result.seedHash.toString() << std::endl;
        std::cout << "mix hash    " << result.mixHash.toString() << std::endl;
        std::cout << "hash        " << result.hash.toString() << std::endl;
        std::cout << "target      " << this->lastWork.target.toString() << std::endl;
        std::cout << "-------------------------------" << std::endl;
    }
}

void PoolConnection::submitHashRate(uint64_t hashrate, Bits<256> identity)
{
    std::string httpBody = "{\"jsonrpc\":\"2.0\", \"method\":\"eth_submitHashrate\", \"params\":[";
    httpBody += "\"0x"+hexstr<uint64_t>(hashrate)+"\",";
    httpBody += "\""+identity.toString()+"\"";
    httpBody += "],\"id\":73}";
    
    this->httpClient.httpPost(this->getPoolUrl(), httpBody);
}