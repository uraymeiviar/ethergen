#include "gpuworker.h"
#include <memory>
#include <iostream>
#include <fstream>
#include "cl/cl.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/error/error.h"

static void clCheckError(cl_int err, const char * name)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << "(OpenCL) ERROR: " <<  name << " (" << err << ")" << std::endl;
    }
}

static void clContextCallback( const char *errinfo,
                               const void *private_info,
                               size_t cb, 
                               void *userData )
{
    GpuWorker* gpuWorker = (GpuWorker*)userData;
    std::cerr << "(OpenCL:" << gpuWorker->getName() << ") ERROR: " << std::string(errinfo) << std::endl;
}

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

void ClDeviceInfo::printInfo()
{
    std::string endian = "Endian:L";
    if(!this->endianLittle)
    {
        endian = "Endian:B";
    }
    std::string wiSize = std::to_string(this->maxWorkItemSize[0]);
    for(size_t w = 1 ; w<this->maxWorkItemSize.size() ; w++)
    {
        wiSize += "x"+std::to_string(this->maxWorkItemSize[w]);
    }
    std::cout << "DeviceID: " << this->device << " Platform: " << this->platform << " " << this->name << " ( " <<ClDeviceInfo::deviceTypeToString(this->type) << ") " << std::endl;
    
    std::cout << "  " << this->version << "/ SW." << this->driverVersion << " " << this->vendor << " (0x" << std::hex << this->vendorId << ") " << endian << std::endl;
    
    std::cout << "  GMemSize: " << std::to_string(this->globalMemorySize/(1024*1024)) << "MB" <<
    "  GCacheSize: " << std::to_string(this->globalMemoryCacheSize) <<
    "  GCacheLine: " << std::to_string(this->globalMemoryCacheLineSize/1024) << "KB" <<
    "  LMemSize: " << std::to_string(this->localMemorySize/1024) << "KB" << std::endl;
    
    std::cout << "  MaxClock: " << std::to_string(this->maxClock) << "MHz" <<
    "  MaxCU: " << std::to_string(this->maxComputeUnit) <<
    "  MaxConst: " << std::to_string(this->maxConstant) << "x" << std::to_string(this->maxConstantBufferSize/1024) << "KB" <<
    "  MaxAlloc: " << std::to_string(this->maxMemoryAllocSize/(1024*1024)) << "MB" << std::endl;
    std::cout << "  MaxParamSize: " << std::to_string(this->maxParameterSize) <<
    "  MaxWGSize: " << std::to_string(this->maxWorkGroupSize) <<
    "  MaxWISize: " << wiSize << std::endl << std::endl;
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

GpuWorker::GpuWorker()
{
    this->clContext = nullptr;
    this->buffer.resize(GpuWorker::defaultWorkSize*4);
}

void GpuWorker::setParam(std::string param)
{
    Worker::setParam(param);
    rapidjson::Document configJson;
    rapidjson::ParseResult parseResult = configJson.Parse(param.c_str());
    if(parseResult)
    {
        if(configJson.HasMember("worksize") && configJson["worksize"].IsInt())
        {
            size_t worksize = configJson["worksize"].GetInt();
            worksize = 64*(worksize/64);
            this->buffer.resize(worksize*4);
        }
        
        std::string kernel = "";
        if(configJson.HasMember("kernel") && configJson["kernel"].IsString())
        {
            kernel = configJson["kernel"].GetString();
        }
        
        if(configJson.HasMember("deviceid") && configJson["deviceid"].IsString())
        {
            std::string deviceIdStr = configJson["deviceid"].GetString();
            size_t deviceId = 0;
            if(deviceIdStr.substr(0,2) == "0x")
            {
                deviceId = std::stoull(deviceIdStr.substr(2,deviceIdStr.length()-2),nullptr,16);
            }
            else
            {
                deviceId = std::stoull(deviceIdStr);
            }
            this->initDevices((cl_device_id)deviceId, kernel);
        }
        else{
            this->initDevices(0, kernel);
        }
    }
    else
    {
        std::cout << "error while parsing gpu worker params" << std::endl;
        std::cout << rapidjson::GetParseError_En(parseResult.Code()) <<
        ":" << std::to_string(parseResult.Offset()) << std::endl;
    }
}

bool GpuWorker::initDevices(cl_device_id deviceId, std::string kernelFile = "")
{
    ClDeviceInfo deviceInfo;
    
    std::map<cl_platform_id,std::vector<cl_device_id>> devices;
    if(deviceId != 0)
    {
        bool found = false;
        GpuWorker::getAllDevices(devices);
        for(auto& pair : devices)
        {
            for(auto& id : pair.second)
            {
                if(id == deviceId){
                    found = true;
                    deviceInfo.device = id;
                    deviceInfo.platform = pair.first;
                    break;
                }
            }
        }
        if(!found)
        {
            std::cout << "OpenCL Device with ID " << deviceId << " is not found" << std::endl;
            std::cout << "please query deviceId with \"ethergen clinfo\" command" << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << "OpenCL DeviceID is not configured in config file!" << std::endl;
        std::cout << "attempting to find default GPU to use, if you have more than one OpenCL Device" << std::endl;
        std::cout << "please state each of them as separate worker using deviceId in config file" << std::endl;
        std::cout << "to get list of OpenCL devices query it with \"ethergen clinfo\" command" << std::endl;
        GpuWorker::getAllDevices(devices,CL_DEVICE_TYPE_GPU);
        if(devices.size() > 0 && devices.begin()->second.size() > 0)
        {
            deviceInfo.device = devices.begin()->second[0];
            deviceInfo.platform = devices.begin()->first;
        }
        else
        {
            std::cout << std::endl;
            std::cout << "No OpenCL Device Found. " << std::endl;
            return false;
        }
    }
    
    if(ClDeviceInfo::getInfo(deviceInfo))
    {
        deviceInfo.printInfo();
    }
    
    std::ifstream inputFile( kernelFile, std::ios::binary );
    if(inputFile.fail())
    {
        std::cerr << "unable to open cl kernel file " << kernelFile << std::endl;
        return false;
    }
    else
    {
        std::string kernelCode((std::istreambuf_iterator<char>(inputFile)),
                                std::istreambuf_iterator<char>());
        inputFile.close();
        
        cl_context_properties contextProperties[] =
        {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)deviceInfo.platform,
            0
        };
        
        cl_int errNum;
        this->clContext = clCreateContext(
                                          contextProperties,
                                          1,
                                          &deviceInfo.device,
                                          clContextCallback,
                                          this,
                                          &errNum);
        clCheckError(errNum,"clCreateContext");
        if(errNum == CL_SUCCESS)
        {
            size_t kernelCodeLength = kernelCode.length();
            const char* kernelCodeStr = kernelCode.c_str();
            cl_program program = clCreateProgramWithSource(
                                                           this->clContext,
                                                           1,
                                                           &kernelCodeStr,
                                                           &kernelCodeLength,
                                                           &errNum);
            clCheckError(errNum, "clCreateProgramWithSource");
            if(errNum == CL_SUCCESS)
            {
                errNum = clBuildProgram(program,
                                        1,
                                        &deviceInfo.device,
                                        "-I.",
                                        NULL,
                                        NULL);
                if (errNum != CL_SUCCESS)
                {
                    size_t logSize = 0;
                    clGetProgramBuildInfo(program,
                                          deviceInfo.device,
                                          CL_PROGRAM_BUILD_LOG,
                                          0,
                                          nullptr,
                                          &logSize);
                    
                    std::vector<char> log;
                    log.resize(logSize+1);
                    clGetProgramBuildInfo(program,
                                          deviceInfo.device,
                                          CL_PROGRAM_BUILD_LOG,
                                          log.size(),
                                          log.data(),
                                          NULL);
                    
                    std::cerr << "Error in OpenCL C source: " << std::endl;
                    std::cerr << log.data() << std::endl;
                    clCheckError(errNum, "clBuildProgram");
                    clReleaseContext(this->clContext);
                    this->clContext = nullptr;
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else
            {
                clReleaseContext(this->clContext);
                this->clContext = nullptr;
                return false;
            }
        }
        this->clContext = nullptr;
        return false;
    }
}

GpuWorker::~GpuWorker()
{
    if(this->clContext != nullptr)
    {
        clReleaseContext(this->clContext);
        this->clContext = nullptr;
    }
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