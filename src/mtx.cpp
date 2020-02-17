#include <cassert>
#include <random>
#include <stdexcept>
#ifdef USE_SYSV_SHM
#include <sys/shm.h>
#include <sys/stat.h>
#else
#include <sys/mman.h>
#endif

#define _LOG_SRC "MTX"
#include "log.h"

#include "mtx.h"

static
void get_indexes(size_t              mtx_size,
                 size_t              num_workers,
                 size_t              worker_idx,
                 size_t * __restrict start_row,
                 size_t * __restrict end_row) {
    assert(start_row != nullptr);
    assert(end_row != nullptr);
    *start_row = 0;
    *end_row = 0;
    if ((mtx_size == 0) || (num_workers == 0)) return;
    size_t rows_per_worker = mtx_size / num_workers;
    // не последний воркер (последнему достаётся больше всех если не делится нацело)
    if (worker_idx != (num_workers - 1)) {
        *start_row = worker_idx*rows_per_worker;
        *end_row   = (worker_idx + 1)*rows_per_worker;
    } else {
        *start_row = worker_idx*rows_per_worker;
        *end_row = mtx_size;
    }
}

static
void mtxmul_square(const int64_t * __restrict a,
                   const int64_t * __restrict b,
                         int64_t * __restrict c,
                   size_t                     size,
                   size_t                     num_workers,
                   unsigned int               worker_idx) {
    size_t start_row, end_row;
    get_indexes(size, num_workers, worker_idx, &start_row, &end_row);
    for (size_t i = start_row ; i < end_row; i++) {
        for (size_t j = 0; j < size; j++) {
            int64_t s = 0;
            for (size_t r = 0 ; r < size; r++) {
                s += a[i*size + r]*b[r*size + j];
            }
            c[i*size + j] = s;
        }
    }
}

#ifdef USE_SYSV_SHM

SharedMem::SharedMem(size_t size) : m_size(size), m_shmid(-1), m_addr((void *)-1) {
    m_shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR);
    if (m_shmid == -1) {
        _LOG_ERROR_ERRNO("shmget() failed");
        throw std::runtime_error("shmget() failed");
    }
    _LOG_TRACE("shared memory segment `%d` created", m_shmid);
}

SharedMem::~SharedMem() {
    Detach();
    if (shmctl(m_shmid, IPC_RMID, nullptr) == -1) {
        _LOG_ERROR_ERRNO("shmctl() failed");
    }
    _LOG_TRACE("shared memory segment `%d` destroyed", m_shmid);
}

void *SharedMem::Attach() {
    // значение адреса нам не важно, поэтому аргумент addr = NULL
    m_addr = shmat(m_shmid, nullptr, 0);
    if (m_addr == (void *)-1) {
        _LOG_ERROR_ERRNO("shmat() failed");
        throw std::runtime_error("shmat() failed");
    }
    _LOG_TRACE("shared memory segment `%d` attached to addr `%p`", m_shmid, m_addr);
    // заметим, что после fork() m_addr продолжить существовать в дочернем процессе,
    // и он останется разделяемым с родителем (т.е. запись в него со стороны предка будет видна со стороны родителя)
    return m_addr;
}

void SharedMem::Detach() noexcept {
    if (m_addr != (void *)-1) {
        if (shmdt(m_addr) == -1) {
            _LOG_ERROR_ERRNO("shmdt() failed");
        } else {
            _LOG_TRACE("shared memory segment `%d` detached from addr `%p`", m_shmid, m_addr);
            m_addr = (void *)-1;
        }
    }
}

#else

SharedMem::SharedMem(size_t size) : m_size(size), m_addr(MAP_FAILED) {
}

SharedMem::~SharedMem() {
    Detach();
}

void *SharedMem::Attach() {
    // значение адреса нам не важно, поэтому аргумент addr = NULL
    m_addr = mmap(NULL, m_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (m_addr == MAP_FAILED) {
        _LOG_ERROR_ERRNO("mmap() failed");
        throw std::runtime_error("mmap() failed");
    }
    _LOG_TRACE("shared memory segment attached to addr `%p`", m_addr);
    // заметим, что после fork() m_addr продолжить существовать в дочернем процессе,
    // и он останется разделяемым с родителем (т.е. запись в него со стороны предка будет видна со стороны родителя)
    return m_addr;
}

void SharedMem::Detach() noexcept {
    if (m_addr != MAP_FAILED) {
        if (munmap(m_addr, m_size) == -1) {
            _LOG_ERROR_ERRNO("shmdt() failed");
        } else {
            _LOG_TRACE("shared memory segment detached from addr `%p`", m_addr);
            m_addr = (void *)-1;
        }
    }
}

#endif

SquareMtx::SquareMtx(size_t size) :m_ptr(nullptr), m_size(size), m_shmem(size*size*sizeof(int64_t)) {
    m_ptr = static_cast<int64_t*>(m_shmem.Attach());
}

SquareMtx::~SquareMtx() {
}

void SquareMtx::fill() {
    std::random_device rd;
    std::mt19937_64 eng(rd());
    std::uniform_int_distribution<int64_t> distr;
    for (size_t i = 0 ; i < (m_size*m_size); i++) {
        m_ptr[i] = distr(eng);
    }
}

void SquareMtx::mul(const SquareMtx &b, SquareMtx &c, size_t num_workers, unsigned int worker_idx) const {
    if ((m_size != b.m_size) || (m_size != c.m_size) || (b.m_size != c.m_size)) {
        throw std::runtime_error("Matrices size are not equal");
    }
    mtxmul_square(m_ptr,
                  b.m_ptr,
                  c.m_ptr,
                  m_size,
                  num_workers,
                  worker_idx);
}

void SquareMtx::mul(const SquareMtx *b, SquareMtx *c, size_t num_workers, unsigned int worker_idx) const {
    if ((m_size != b->m_size) || (m_size != c->m_size) || (b->m_size != c->m_size)) {
        throw std::runtime_error("Matrices size are not equal");
    }
    mtxmul_square(m_ptr,
                  b->m_ptr,
                  c->m_ptr,
                  m_size,
                  num_workers,
                  worker_idx);
}

bool SquareMtx::is_equal(const SquareMtx &other) const {
    if (m_size != other.m_size) return false;
    for (size_t i = 0 ; i < (m_size*m_size); i++) {
        if (m_ptr[i] != other.m_ptr[i]) return false;
    }
    return true;
}

const void *SquareMtx::ptr() const {
    return m_ptr;
}