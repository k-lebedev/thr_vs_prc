#include <cstdio>
#include <pthread.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>

#include "thread_worker.h"
#include "process_worker.h"
#define _LOG_SRC "MAIN"
#include "log.h"
#include "mtx.h"

static
const log_src_descr_t log_srcs[] = {
    {"MAIN"          , LL_TRACE },
    {"THREAD_WORKER" , LL_TRACE },
    {"PROCESS_WORKER", LL_TRACE },
    {"MTX"           , LL_TRACE },
};

static
IWorker *create_worker(const char         *type,
                       const WorkerParams &worker_params) {
    std::string type_str(type);
    std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::toupper);
    if (type_str == "THREAD") {
        return new ThreadWorker(worker_params);
    }
    if (type_str == "PROCESS") {
        return new ProcessWorker(worker_params);
    }
    throw std::runtime_error("unknown worker type");
}

static
void process(const SquareMtx &a,
             const SquareMtx &b,
                   SquareMtx &c,
             size_t           num_workers,
             const char      *worker_type) {
    std::vector<std::unique_ptr<IWorker>> workers;
    for (unsigned int i = 0 ; i < num_workers; i++) {
        const WorkerParams wp = {i, num_workers, &a, &b, &c};
        workers.push_back(std::unique_ptr<IWorker>(create_worker(worker_type, wp)));
    }
}

class Logger {
public:
    Logger() {
        log_init(LL_TRACE, false);
        log_register_ex(log_srcs, TBL_SZ(log_srcs));
    }
    ~Logger() {
        log_destroy();
    }
};

int main(int argc, const char **argv) {
    Logger l;
    if (argc < 4) {
        _LOG_ERROR("must provide at least 3 arguments");
        return -1;
    }
    _LOG_INFO("Hellow!");
    try {
        const char *mtx_sz_str = argv[1];
        const char *num_workers_str = argv[2];
        const char *woker_type = argv[3];
        const size_t mtx_sz = std::stoul(std::string(mtx_sz_str));
        const size_t num_workers = std::stoul(std::string(num_workers_str));
        _LOG_TRACE("creating matrixes...");
        SquareMtx a(mtx_sz);
        SquareMtx b(mtx_sz);
        SquareMtx c(mtx_sz);
        SquareMtx d(mtx_sz);

        a.fill();
        b.fill();

        // посчитаем произведение матриц в одном потоке для контроля правильности
        _LOG_TRACE("single thread/process calculating...");
        const auto start_single = std::chrono::steady_clock::now();
        a.mul(b, d, 1, 0);
        const auto end_single = std::chrono::steady_clock::now();
        const auto single_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_single - start_single).count();
        _LOG_INFO("Finished. Elapsed time = %zu ms", single_ms);
        _LOG_INFO("multiple thread/process calculating...");
        const auto start_multi = std::chrono::steady_clock::now();
        process(a, b, c, num_workers, woker_type);
        const auto end_multi = std::chrono::steady_clock::now();
        const auto multi_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_multi - start_multi).count();
        _LOG_INFO("Finished. Elapsed time = %zu ms, ratio single/multi = %f",
                  multi_ms,
                  (float)single_ms/(float)multi_ms);
        if (!c.is_equal(d)) {
            _LOG_ERROR("matrices are not equal");
        }
    } catch (const std::runtime_error &e) {
        _LOG_ERROR("Exception: %s", e.what());
    }
    _LOG_INFO("Bye!");
    return 0;
}