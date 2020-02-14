#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include <string>

#define _LOG_SRC "PROCESS_WORKER"
#include "log.h"
#include "macros.h"

#include "process_worker.h"

ProcessWorker::ProcessWorker(const WorkerParams &p) : IWorker(p), m_pid(0) {
    m_pid = fork();
    if (m_pid < 0) {
        _LOG_ERROR_ERRNO("worker `%u`: fork() failed", m_params.id);
        throw std::runtime_error("fork() failed");
    }
    if (m_pid) {
        _LOG_TRACE("worker `%u`: created with pid = %ld", m_params.id, (intmax_t)m_pid);
        // parent
        return;
    }
    // child
    p.a->mul(p.b, p.c, p.num_workers, p.id);
    exit(100);
}

static
std::string status_str(int status) {
    std::string res("unknown status");
    if (WIFEXITED(status)) {
        res = "terminated normally: exit status = ";
        res += std::to_string(WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        if (WCOREDUMP(status)) {
            res = "core dumped";
        } else {
            res = "terminated by a signal: signal = ";
            res += std::to_string(WTERMSIG(status));
        }
    } else if (WIFSTOPPED(status)) {
        res = "was stopped by delivery of a signal";
    }
    return res;
}

ProcessWorker::~ProcessWorker() {
    if (m_pid) {
        int status;
        if (waitpid(m_pid, &status, 0) == -1) {
            _LOG_ERROR_ERRNO("worker `%u`: wait(%jd) failed", m_params.id, (intmax_t)m_pid);
        } else {
            _LOG_TRACE("worker `%u` destroyed: %s", m_params.id, status_str(status).c_str());
        }
    }
}


