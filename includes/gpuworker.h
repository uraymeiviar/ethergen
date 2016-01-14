#ifndef gpuworker_h
#define gpuworker_h
#include "worker.hpp"

#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS

#ifdef __APPLE__
#include <OpenCL/OpenCL.h>
#else
#include <CL/CL.h>
#endif

#include <thread>
#include <atomic>
#include "sha3.hpp"

class GpuWorkerFactory : public WorkerFactory
{
public:
    virtual std::unique_ptr<Worker> create(std::string name) override;
};

class ClDeviceInfo
{
public:
    cl_platform_id platform;
    cl_device_id device;
    cl_device_type type;
    cl_bool endianLittle;
    cl_ulong globalMemoryCacheSize;
    cl_ulong globalMemoryCacheLineSize;
    cl_ulong globalMemorySize;
    cl_ulong localMemorySize;
    cl_uint maxClock;
    cl_uint maxComputeUnit;
    cl_uint maxConstant;
    cl_ulong maxConstantBufferSize;
    cl_ulong maxMemoryAllocSize;
    size_t maxParameterSize;
    size_t maxWorkGroupSize;
    cl_uint maxWorkItemDimension;
    std::vector<size_t> maxWorkItemSize;
    std::string name;
    std::string version;
    std::string driverVersion;
    std::string vendor;
    cl_uint vendorId;
    static bool getInfo(ClDeviceInfo& info);
    static std::string deviceTypeToString(cl_device_type type);
};

class GpuHashPad
{
public:
    inline void init(uint64_t nonce,const Bits<256>& headerHash)
    {
        memcpy(this->smix[0].ptr(), headerHash.ptr(), 32);
        memcpy(this->smix[0].ptr()+32, &nonce, 8);
        *this->smix[3].ptr64(8) = nonce;
    }
    inline uint64_t getNonce() const
    {
        return *this->smix[3].ptr64(8);
    }
    inline const uint8_t* getHashResultPtr()
    {
        return this->smix[3].ptr();
    }
    inline const uint8_t* getMixResultPtr()
    {
        return this->smix[1].ptr();
    }
    inline bool isHashResultLowerThan(const Bits<256>& ref)
    {
        return ref > this->smix[3].ptr();
    }
    inline uint32_t fnv(uint32_t a, uint32_t b)
    {
        return (a + (a<<1) + (a<<4) + (a<<7) + (a<<8) + (a<<24)) ^ b;
    }
    inline void preHash()
    {
        SHA3_512(this->smix[0].ptr(), this->smix[0].ptr(), 40);
        memcpy(this->smix[1].ptr(), this->smix[0].ptr(), 64);
        memcpy(this->smix[2].ptr(), this->smix[0].ptr(), 64);
    }
    inline void hash(const std::shared_ptr<WorkGraph> workgraph)
    {
        const uint32_t numPages = (uint32_t)(workgraph->getDAGByteLength() / 128);
        const Bytes<64>* dag = workgraph->getDAG();
        uint32_t* mix = (uint32_t*)smix[1].ptr();
        for (uint32_t i = 0; i != 64; ++i)
        {
            uint32_t index = fnv(smix[0].value32(0)^i, mix[i % 32]) % numPages;
            
            const uint32_t* dagNode = dag[index*2 ].ptr32();
            for (size_t w = 0; w < 32; w++)
            {
                mix[w] = fnv(mix[w],dagNode[w]);
            }
        }
    }
    inline void postHash()
    {
        uint32_t* mix = (uint32_t*)smix[1].ptr();
        for (size_t w = 0; w < 8; w++)
        {
            mix[w] = fnv(mix[w*4],mix[w*4 + 1]);
            mix[w] = fnv(mix[w  ],mix[w*4 + 2]);
            mix[w] = fnv(mix[w  ],mix[w*4 + 3]);
        }
        
        SHA3_256(smix[3].ptr(), smix[0].ptr(), 64 + 32);
    }
    
    Bytes<64> smix[4];
};

class GpuWorker : public Worker
{
public:
    void setWork(const Work& work, uint64_t startNonce ) override;
    virtual void setRun(bool run) override;
    virtual void updateHashrate() override;
    
    static GpuWorkerFactory& getFactory();
    static void getPlatforms(std::vector<cl_platform_id>& platforms);
    static cl_uint getNumPlatforms();
    static void getDevices(cl_platform_id platform,std::vector<cl_device_id>& devices, cl_device_type deviceType = CL_DEVICE_TYPE_ALL);
    static void getAllDevices(std::map<cl_platform_id,std::vector<cl_device_id>>& devices, cl_device_type deviceType = CL_DEVICE_TYPE_ALL);
    static cl_uint getNumDevices(const cl_platform_id& platform,cl_device_type deviceType = CL_DEVICE_TYPE_ALL);
protected:
    std::thread workerThreads;
    std::atomic<uint64_t> totalNonce;
    std::atomic<uint64_t> currentNonce;
    std::mutex setWorkMutex;
    GpuHashPad hashPad[64];
    void threadProc();
};

#endif /* gpuworker_h */
