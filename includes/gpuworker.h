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
    void printInfo();
};

class GpuWorker : public Worker
{
public:
    GpuWorker();
    virtual ~GpuWorker() override;
    static const size_t defaultWorkSize = 4096;
    void setWork(const Work& work, uint64_t startNonce ) override;
    void setParam(std::string param) override;
    virtual void setRun(bool run) override;
    virtual void updateHashrate() override;
    
    static GpuWorkerFactory& getFactory();
    static void getPlatforms(std::vector<cl_platform_id>& platforms);
    static cl_uint getNumPlatforms();
    static void getDevices(cl_platform_id platform,std::vector<cl_device_id>& devices, cl_device_type deviceType = CL_DEVICE_TYPE_ALL);
    static void getAllDevices(std::map<cl_platform_id,std::vector<cl_device_id>>& devices, cl_device_type deviceType = CL_DEVICE_TYPE_ALL);
    static cl_uint getNumDevices(const cl_platform_id& platform,cl_device_type deviceType = CL_DEVICE_TYPE_ALL);
protected:
    bool initDevices(cl_device_id deviceId, std::string kernelFile);
    std::thread workerThreads;
    std::atomic<uint64_t> totalNonce;
    std::atomic<uint64_t> currentNonce;
    std::mutex setWorkMutex;
    std::vector<Bytes<64>> buffer;
    cl_context clContext;
    void threadProc();
};

#endif /* gpuworker_h */
