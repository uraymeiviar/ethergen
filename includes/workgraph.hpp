#ifndef workgraph_h
#define workgraph_h
#include "block.hpp"
#include <map>
#include <vector>
#include "bits.hpp"

class WorkGraph : public Block
{
public:
    static const uint32_t DatasetParent = 256;
    static const uint32_t FnvPrime = 0x01000193;
    void writeDAGToFile(std::string path);
    void writeDAGCacheToFile(std::string path);
    bool readDAGFromFile(std::string path);
    bool readDAGCacheFromFile(std::string path);
    const Bytes<64>* getDAG() const;
    const Bytes<64>* getDAGCache() const;
    size_t getDAGByteLength() const;
    size_t getDAGCacheByteLength() const;
    
    WorkGraph() : Block() {};
    WorkGraph(const WorkGraph& ref);
    WorkGraph(const WorkGraph& ref, uint64_t blockNumber);
    WorkGraph(uint64_t blockNumber);
    WorkGraph(uint64_t blockNumber,Bits<256> seedHash);
    WorkGraph(uint64_t blockNumber,std::string dagFile,std::string dagCacheFile);
    
    static uint32_t getCacheSize(uint64_t blockNumber);
    static uint64_t getDAGSize(uint64_t blockNumber);
    static uint32_t fnvHash(uint32_t x, uint32_t y);
    static void deleteWorkGraph(uint64_t blockNumber);
    static void deleteWorkGraph(uint64_t blockNumber,std::string dagFile, std::string dagCacheFile);
    static std::shared_ptr<WorkGraph> getWorkGraph(uint64_t blockNumber);
    static std::shared_ptr<WorkGraph> getWorkGraph(uint64_t blockNumber, std::string dagFile, std::string dagCacheFile);
protected:
    std::vector<Bytes<64>> cache;
    std::vector<Bytes<64>> dag;
    void computeCache();
    void computeDAGItem(Bytes<64>& target, size_t index);
    void computeDAGItem(Bytes<64>& target, size_t index, const std::vector<Bytes<64>>& cache);
    void computeDAG();
    static void registerWorkGraph(std::shared_ptr<WorkGraph>, uint64_t blockNumber);
    static std::map<uint64_t,std::shared_ptr<WorkGraph>> cachedWorkGraph;
};

#endif /* workgraph_h */
