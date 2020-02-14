#include <stdexcept>

#define _LOG_SRC "THREAD_WORKER"
#include "log.h"
#include "macros.h"

#include "thread_worker.h"

static
void *worker_routine(void *arg) {
    IWorker *w = static_cast<IWorker*>(arg);
    const WorkerParams &p = w->get_params();
    p.a->mul(p.b, p.c, p.num_workers, p.id);
    return nullptr;
}

ThreadWorker::ThreadWorker(const WorkerParams &p) :IWorker(p), m_thread(0) {
    int err = pthread_create(&m_thread, nullptr, worker_routine, this);
    if (err != 0) {
        _LOG_ERROR_ERRNO2("worker `%u`: pthread_create() failed", m_params.id, err);
        throw std::runtime_error("pthread_create() failed");
    }
    _LOG_TRACE("worker `%u`: created with thread id = %lu", m_params.id, m_thread);
}

ThreadWorker::~ThreadWorker() {
    if (m_thread) {
        int err = pthread_join(m_thread, nullptr);
        if (err != 0) {
            _LOG_ERROR_ERRNO2("worker `%u`: pthread_join failed()", err, m_params.id);
        } else {
            _LOG_TRACE("worker `%u` destroyed", m_params.id);
        }
    }
}