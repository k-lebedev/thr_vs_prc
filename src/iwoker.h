#pragma once

#include "mtx.h"

struct WorkerParams {
    unsigned int     id;
    size_t           num_workers;
    const SquareMtx *a;
    const SquareMtx *b;
          SquareMtx *c;
};

class IWorker {
public:
    explicit IWorker(const WorkerParams &p)
    : m_params(p) {}
    virtual ~IWorker()  = default;
    const WorkerParams& get_params() const {
        return m_params;
    }
protected:
    WorkerParams m_params;
};
