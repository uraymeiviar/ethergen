#include "miner.hpp"

Miner::Miner(Bits<160> accountId)
{
    this->accountId = accountId;
    this->running = false;
}

Miner::~Miner()
{
    this->setRun(false);
}

const Bits<160>& Miner::getAccountId() const
{
    return this->accountId;
}

bool Miner::isRunning() const
{
    return this->running;
}

void Miner::setRun(bool run)
{
    if(this->running != run)
    {
        this->running = run;
        if(run)
        {
            for(size_t i=0 ; i<this->workers.size() ; i++)
            {
                this->workers.at(i).second->requestNewWork();
                this->workers.at(i).first->setRun(run);
            }
        }
        else
        {
            for(size_t i=0 ; i<this->workers.size() ; i++)
            {
                this->workers.at(i).first->setRun(run);
            }
        }
    }
}

size_t Miner::countWorker() const
{
    return this->workers.size();
}

const Worker* Miner::getWorker(size_t index) const
{
    if(index < this->workers.size())
    {
        return this->workers.at(index).first.get();
    }
    else
    {
        return nullptr;
    }
}

const Connection* Miner::getWorkerConnection(size_t index) const
{
    if(index < this->workers.size())
    {
        return this->workers.at(index).second.get();
    }
    else
    {
        return nullptr;
    }
}

void Miner::destroyWorker(size_t index)
{
    if(index < this->workers.size())
    {
        this->workers.at(index).first->setRun(false);
        this->workers.erase(this->workers.begin()+index);
    }
}

uint64_t Miner::getCurrentHashrate() const
{
    uint64_t hashrate = 0;
    for(size_t i=0 ; i<this->workers.size() ; i++)
    {
        hashrate += this->workers.at(i).first->getCurrentHashrate();
    }
    return hashrate;
}

void Miner::addWorker(WorkerFactory& workerFactory, ConnectionFactory& connectionFactory, std::string name, std::string endpoint)
{
    std::unique_ptr<Worker> worker = workerFactory.create(name);
    std::unique_ptr<Connection> connection = connectionFactory.create(this->accountId, name, endpoint);

    connection->onNewWork = std::bind(&Miner::eventOnNewWork,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,worker.get());
    worker->onValidResult = std::bind(&Miner::eventWorkResult,this, std::placeholders::_1,
                                      std::placeholders::_2,connection.get());
    if(this->running)
    {
        worker->setRun(this->running);
    }
    
    this->workers.push_back(std::make_pair(std::move(worker),std::move(connection)));
}

void Miner::eventOnNewWork(Connection& connection, const Work& work, uint64_t startNonce,Worker* worker)
{
    worker->setWork(work,startNonce);
}

void Miner::eventWorkResult(const Worker& worker, WorkResult result, Connection* connection)
{
    connection->submitResult(result);
    connection->requestNewWork();
}