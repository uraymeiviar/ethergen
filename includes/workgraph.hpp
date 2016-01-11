#ifndef workgraph_h
#define workgraph_h
#include "block.hpp"
#include <map>
#include <vector>
#include "bits.hpp"

class WorkGraph
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
    Epoch getEpoch() const;
    const Bits<256> getSeedHash() const;
    
    WorkGraph(Epoch epoch);
    WorkGraph(Epoch epoch,std::string dagFile,std::string dagCacheFile);
    
    static uint32_t getDAGCacheSize(Epoch epoch);
    static uint64_t getDAGSize(Epoch epoch);
    static uint32_t fnvHash(uint32_t x, uint32_t y);
    static void deleteWorkGraph(Epoch epoch);
    static void deleteWorkGraph(Epoch epoch,std::string dagFile, std::string dagCacheFile);
    static std::shared_ptr<WorkGraph> getWorkGraph(Epoch epoch);
    static std::shared_ptr<WorkGraph> getWorkGraph(Epoch epoch, std::string dagFile, std::string dagCacheFile);
protected:
    Epoch epoch;
    Bits<256> seedHash;
    std::vector<Bytes<64>> cache;
    std::vector<Bytes<64>> dag;
    void computeCache();
    void computeDAGItem(Bytes<64>& target, size_t index);
    void computeDAGItem(Bytes<64>& target, size_t index, const std::vector<Bytes<64>>& cache);
    void computeDAG();
    static void registerWorkGraph(std::shared_ptr<WorkGraph>, Epoch epoch);
    static std::map<Epoch,std::shared_ptr<WorkGraph>> cachedWorkGraph;
};

#endif /* workgraph_h */
