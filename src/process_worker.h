#pragma once

#include <sys/types.h>

#include "iwoker.h"

class ProcessWorker : public IWorker {
public:
    ProcessWorker();
    ~ProcessWorker();
private:
    pid_t m_pid;
};