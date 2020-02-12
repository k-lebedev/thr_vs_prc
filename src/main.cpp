#include <cstdio>
#include <pthread.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <vector>


#define _LOG_SRC "MAIN"
#include "log.h"

static
const log_src_descr_t log_srcs[] = {
        {"MAIN" , LL_INFO },
        {"OLOLO", LL_INFO }
};

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

int main() {
    Logger l;
    _LOG_INFO("Hellow!");
    _LOG_INFO("Bye!");
    return 0;
}