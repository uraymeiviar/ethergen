#include "worker.hpp"

void Worker::setWork(const Work& work, uint64_t startNonce )
{
    bool lastState = this->isRunning();
    this->setRun(false);
    this->work = Work(work);
    this->workResult.nonce = startNonce;
    this->workResult.blockNumber = work.getBlockNumber();
    this->workResult.target = work.target;
    this->workResult.headerHash = work.headerHash;
    this->workResult.seedHash = work.getSeedHash();
    this->workGraph = WorkGraph::getWorkGraph(work.getBlockNumber());
    this->setRun(lastState);
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

Worker::Worker()
{
    this->running = false;
    this->currentHashrate = 0;
    this->workResult.nonce = 0;
    this->name = "worker";
}

Worker::~Worker()
{
    this->setRun(false);
}