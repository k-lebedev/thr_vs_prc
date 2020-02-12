#include <stdexcept>

#define _LOG_SRC "THREAD_WORKER"
#include "log.h"
#include "macros.h"

#include "thread_worker.h"

static
void *worker_routine(void *arg) {
    UNUSED_PARAM(arg);
    return nullptr;
}

ThreadWorker::ThreadWorker() {
    int err = pthread_create(&m_thread, nullptr, worker_routine, NULL);
    if (err != 0) {
        _LOG_ERROR_ERRNO2("pthread_create() failed", err);
        throw std::runtime_error("pthread_create() failed");
    }
    _LOG_TRACE("worker `%lu` created", m_thread);
}

ThreadWorker::~ThreadWorker() {
    if (m_thread) {
        int err = pthread_join(m_thread, nullptr);
        if (err != 0) {
            _LOG_ERROR_ERRNO2("pthread_join failed()", err);
        } else {
            _LOG_TRACE("worker `%lu` destroyed", m_thread);
        }
    }
}