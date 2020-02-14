#pragma once

#include <sys/types.h>

#include "iwoker.h"

class ProcessWorker : public IWorker {
public:
    explicit ProcessWorker(const WorkerParams &p);
    ~ProcessWorker() override;
private:
    pid_t m_pid;
};