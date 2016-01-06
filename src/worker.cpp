#include "worker.hpp"

void Worker::setWork(const Work& work, uint64_t startNonce )
{
    bool lastState = this->isRunning();
    this->setRun(false);
    this->work = Work(work);
    this->currentNonce = startNonce;
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
    return this->currentNonce;
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
    this->currentNonce = 0;
    this->name = "worker";
}

Worker::~Worker()
{
    this->setRun(false);
}