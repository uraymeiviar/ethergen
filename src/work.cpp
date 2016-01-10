#include "work.hpp"
#include "sha3.hpp"
#include <iostream>
#include <stdio.h>

bool Work::checkNonce(uint64_t nonce, WorkResult& result, std::shared_ptr<const WorkGraph> graph)
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
    
    size_t numPages = graph->getDAGByteLength() / 128;
    
    for (uint32_t i = 0; i != 64; ++i)
    {
        uint32_t index = WorkGraph::fnvHash(smix[0].value32(0) ^ i, mix[i % 32]) % numPages;
        
        for (size_t n = 0; n != 2; ++n)
        {
            const Bytes<64>* dagNode = &graph->getDAG()[2 * index + n];
            
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
