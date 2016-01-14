#include "gpuworker.h"
#include <memory>
#include "cl/cl.h"

std::string ClDeviceInfo::deviceTypeToString(cl_device_type type)
{
    std::string result;
    if(type & CL_DEVICE_TYPE_CPU)
    {
        result += "CPU ";
    }
    if(type & CL_DEVICE_TYPE_GPU)
    {
        result += "GPU ";
    }
    if(type & CL_DEVICE_TYPE_ACCELERATOR)
    {
        result += "Accelerator ";
    }
    if(type & CL_DEVICE_TYPE_DEFAULT)
    {
        result += "Default ";
    }
    return result;
}

bool ClDeviceInfo::getInfo(ClDeviceInfo& info)
{
    cl_int result = CL_SUCCESS;
    result |= clGetDeviceInfo(info.device,CL_DEVICE_ENDIAN_LITTLE,sizeof(info.endianLittle),&info.endianLittle,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_TYPE,sizeof(info.type),&info.type,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,sizeof(info.globalMemoryCacheSize),&info.globalMemoryCacheSize,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,sizeof(info.globalMemoryCacheLineSize),&info.globalMemoryCacheLineSize,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_GLOBAL_MEM_SIZE,sizeof(info.globalMemorySize),&info.globalMemorySize,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_LOCAL_MEM_SIZE,sizeof(info.localMemorySize),&info.localMemorySize,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_CLOCK_FREQUENCY,sizeof(info.maxClock),&info.maxClock,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(info.maxComputeUnit),&info.maxComputeUnit,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_CONSTANT_ARGS,sizeof(info.maxConstant),&info.maxConstant,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,sizeof(info.maxConstantBufferSize),&info.maxConstantBufferSize,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_MEM_ALLOC_SIZE,sizeof(info.maxMemoryAllocSize),&info.maxMemoryAllocSize,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_PARAMETER_SIZE,sizeof(info.maxParameterSize),&info.maxParameterSize,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(info.maxWorkGroupSize),&info.maxWorkGroupSize,nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,sizeof(info.maxWorkItemDimension),&info.maxWorkItemDimension,nullptr);
    info.maxWorkItemSize.resize(info.maxWorkItemDimension);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_MAX_WORK_ITEM_SIZES,sizeof(size_t)*info.maxWorkItemDimension,info.maxWorkItemSize.data(),nullptr);
    result |= clGetDeviceInfo(info.device,CL_DEVICE_VENDOR_ID,sizeof(info.vendorId),&info.vendorId,nullptr);
    
    size_t bufferRequiredSize = 0;
    clGetDeviceInfo(info.device,CL_DEVICE_NAME,0,nullptr,&bufferRequiredSize);
    char deviceNameBuffer[bufferRequiredSize+1];
    clGetDeviceInfo(info.device,CL_DEVICE_NAME,bufferRequiredSize,deviceNameBuffer,nullptr);
    info.name = std::string((char*)deviceNameBuffer);
    
    clGetDeviceInfo(info.device,CL_DEVICE_VERSION,0,nullptr,&bufferRequiredSize);
    char deviceVersionBuffer[bufferRequiredSize+1];
    clGetDeviceInfo(info.device,CL_DEVICE_VERSION,bufferRequiredSize,deviceVersionBuffer,nullptr);
    info.version = std::string((char*)deviceVersionBuffer);
    
    clGetDeviceInfo(info.device,CL_DRIVER_VERSION,0,nullptr,&bufferRequiredSize);
    char driverVersionBuffer[bufferRequiredSize+1];
    clGetDeviceInfo(info.device,CL_DRIVER_VERSION,bufferRequiredSize,driverVersionBuffer,nullptr);
    info.driverVersion = std::string((char*)driverVersionBuffer);
    
    clGetDeviceInfo(info.device,CL_DEVICE_VENDOR,0,nullptr,&bufferRequiredSize);
    char vendorBuffer[bufferRequiredSize+1];
    clGetDeviceInfo(info.device,CL_DEVICE_VENDOR,bufferRequiredSize,vendorBuffer,nullptr);
    info.vendor = std::string((char*)vendorBuffer);
    
    return result == CL_SUCCESS;
}

std::unique_ptr<Worker> GpuWorkerFactory::create(std::string name)
{
    return std::make_unique<GpuWorker>();
}

GpuWorkerFactory& GpuWorker::getFactory()
{
    static GpuWorkerFactory factory;
    return factory;
}

void GpuWorker::getPlatforms(std::vector<cl_platform_id>& platforms)
{
    cl_uint platformCount = GpuWorker::getNumPlatforms();
    clGetPlatformIDs(0, NULL, &platformCount);
    
    if(platformCount > 0)
    {
        platforms.resize(platformCount);
        clGetPlatformIDs(platformCount, platforms.data(), NULL);
    }
}

cl_uint GpuWorker::getNumPlatforms()
{
    cl_uint platformCount = 0;
    clGetPlatformIDs(0, NULL, &platformCount);
    return platformCount;
}

void GpuWorker::getDevices(cl_platform_id platform,std::vector<cl_device_id>& devices,cl_device_type deviceType)
{
    cl_uint deviceCount = GpuWorker::getNumDevices(platform,deviceType);
    devices.resize(deviceCount);
    clGetDeviceIDs(platform,deviceType,deviceCount, devices.data(), nullptr);
}

void GpuWorker::getAllDevices(std::map<cl_platform_id,std::vector<cl_device_id>>& devices,cl_device_type deviceType)
{
    std::vector<cl_platform_id> platforms;
    GpuWorker::getPlatforms(platforms);
    
    size_t deviceCount = 0;
    for(size_t p=0 ; p<platforms.size() ; p++)
    {
        deviceCount += GpuWorker::getNumDevices(platforms[p],deviceType);
    }
    
    for(size_t p=0 ; p<platforms.size() ; p++)
    {
        std::vector<cl_device_id> platformDevices;
        GpuWorker::getDevices(platforms[p],platformDevices,deviceType);
        devices.insert(std::make_pair(platforms[p],platformDevices));
    }
}

cl_uint GpuWorker::getNumDevices(const cl_platform_id& platform,cl_device_type deviceType)
{
    cl_uint deviceCount = 0;
    clGetDeviceIDs(platform,deviceType,0, NULL, &deviceCount);
    return deviceCount;
}

void GpuWorker::setWork(const Work& work, uint64_t startNonce )
{
    std::lock_guard<std::mutex> lock(this->setWorkMutex);
    Worker::setWork(work,startNonce);
    this->currentNonce = startNonce;
}

void GpuWorker::setRun(bool run)
{
    if(this->running == false && run == true)
    {
        this->running = true;
        this->workerThreads = std::thread(&GpuWorker::threadProc,this);
    }
    else if(this->running == true && run == false)
    {
        this->running = false;
        if(this->workerThreads.joinable())
        {
            this->workerThreads.join();
        }
    }
}

void GpuWorker::updateHashrate()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::duration<float> deltaSecs = std::chrono::duration_cast<std::chrono::duration<float>>(now - this->lastNonceCheckTime);
    this->currentHashrate = this->totalNonce/deltaSecs.count();
    this->lastNonceCheckTime = now;
    this->totalNonce = 0;
}

void GpuWorker::threadProc()
{

}