#pragma once

#include <cstdint>
#include <cstddef>

class SharedMem {
public:
    explicit SharedMem(size_t size);
    ~SharedMem();
    void *Attach();
    void Detach() noexcept;
private:
    size_t  m_size;
#ifdef USE_SYSV_SHM
    int     m_shmid;
#endif
    void   *m_addr;
};

class SquareMtx {
public:
    explicit SquareMtx(size_t size);
    ~SquareMtx();
    void fill();
    void mul(const SquareMtx &b, SquareMtx &c, size_t num_workers, unsigned int worker_idx) const;
    void mul(const SquareMtx *b, SquareMtx *c, size_t num_workers, unsigned int worker_idx) const;
    bool is_equal(const SquareMtx &other) const;
    const void *ptr() const;
    size_t size() const {
        return m_size;
    }
private:
    int64_t   *m_ptr;
    size_t     m_size;
    SharedMem  m_shmem;
};