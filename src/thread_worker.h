#pragma once

#include <pthread.h>

#include "iwoker.h"

class ThreadWorker : public IWorker {
public:
    ThreadWorker();
    ~ThreadWorker();
private:
    pthread_t m_thread;
};
