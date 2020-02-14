#pragma once

#include <pthread.h>

#include "iwoker.h"

class ThreadWorker : public IWorker {
public:
    explicit ThreadWorker(const WorkerParams &p);
    ~ThreadWorker() override;
private:
    pthread_t m_thread;
};
