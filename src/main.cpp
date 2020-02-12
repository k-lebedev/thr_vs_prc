#include <cstdio>
#include <pthread.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <vector>

#include "thread_worker.h"
#include "process_worker.h"
#define _LOG_SRC "MAIN"
#include "log.h"

static
const log_src_descr_t log_srcs[] = {
    {"MAIN"          , LL_INFO  },
    {"THREAD_WORKER" , LL_TRACE },
    {"PROCESS_WORKER", LL_TRACE },
};

static
IWorker *create_worker(const char *type) {
    std::string type_str(type);
    std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::toupper);
    if (type_str == "THREAD") {
        return new ThreadWorker();
    }
    if (type_str == "PROCESS") {
        return new ProcessWorker();
    }
    throw std::runtime_error("unknown worker type");
}

static
void process() {
    std::vector<std::unique_ptr<IWorker>> workers;
    for (size_t i = 0 ; i < 1000; i++) {
        workers.push_back(std::unique_ptr<IWorker>(create_worker("THREAD")));
    }
}

class Logger {
public:
    Logger() {
        log_init(LL_INFO, false);
        log_register_ex(log_srcs, TBL_SZ(log_srcs));
    }
    ~Logger() {
        log_destroy();
    }
};

int main() {
    Logger l;
    _LOG_INFO("Hellow!");
    const auto start = std::chrono::steady_clock::now();
    process();
    const auto end = std::chrono::steady_clock::now();
    _LOG_INFO("Finished. Elapsed time = %zu ms",
              std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    _LOG_INFO("Bye!");
    return 0;
}