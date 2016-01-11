#include "worker.hpp"
#include "sha3.hpp"

void Worker::setWork(const Work& work, uint64_t startNonce )
{
    this->updateHashrate();
    this->bestResult = Bits<256>("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    this->work = Work(work);
    this->workResult.nonce = startNonce;
    this->workResult.blockNumber = work.getBlockNumber();
    this->workResult.target = work.target;
    this->workResult.headerHash = work.headerHash;
    this->workResult.seedHash = work.getSeedHash();
    this->workGraph = WorkGraph::getWorkGraph(work.getEpoch());
}

void Worker::setParam(std::string param)
{
    this->param = param;
}

void Worker::setTarget(const Bits<256>& target)
{
    this->work.target = target;
}

void Worker::updateHashrate()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::duration<float> deltaSecs = std::chrono::duration_cast<std::chrono::duration<float>>(now - this->lastNonceCheckTime);
    this->currentHashrate = this->totalNonce/deltaSecs.count();
    this->lastNonceCheckTime = now;
    this->totalNonce = 0;
}

void Worker::setRun(bool run)
{
    this->running = run;
}

bool Worker::isRunning() const
{
    return this->running;
}

uint64_t Worker::getCurrentNonce() const
{
    return this->workResult.nonce;
}

uint64_t Worker::getCurrentHashrate() const
{
    return this->currentHashrate;
}

const Work& Worker::getCurrentWork() const
{
    return this->work;
}

void Worker::updateNextNonce()
{
    this->workResult.nonce++;
    this->totalNonce++;
}

Worker::Worker()
{
    this->lastNonceCheckTime = std::chrono::system_clock::now();
    this->running = false;
    this->currentHashrate = 0;
    this->workResult.nonce = 0;
    this->name = "worker";
    uint8_t ident[32];
    SHA3_256(ident,(uint8_t*)this->name.c_str(),this->name.length());
    this->identity = Bits<256>(ident);
}

Worker::~Worker()
{
    this->setRun(false);
}

std::string Worker::getName() const
{
    return this->name;
}

Bits<256> Worker::getIdentity() const
{
    return this->identity;
}

Bits<256> Worker::getBestResult() const
{
    return this->bestResult;
}