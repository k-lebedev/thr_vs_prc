#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

#define _LOG_SRC "PROCESS_WORKER"
#include "log.h"
#include "macros.h"

#include "process_worker.h"

static
void worker_routine() {
}

ProcessWorker::ProcessWorker() :m_pid(0){
    m_pid = fork();
    if (m_pid < 0) {
        _LOG_ERROR_ERRNO("fork() failed");
        throw std::runtime_error("fork() failed");
    }
    if (m_pid) {
        _LOG_TRACE("worker `%jd` created", (intmax_t)m_pid);
        // parent
        return;
    }
    // child
    worker_routine();
    exit(1);
}

ProcessWorker::~ProcessWorker() {
    if (m_pid) {
        if (waitpid(m_pid, NULL, 0) == -1) {
            _LOG_ERROR_ERRNO("wait(%jd) failed", (intmax_t)m_pid);
        } else {
            _LOG_TRACE("worker `%jd` destroyed", (intmax_t)m_pid);
        }
    }
}


