//===----------------------------------------------------------------------===//
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifdef __AMP_CPU__
#include <amp_cpu_manage.h>
#else

#pragma once

#include <amp_allocator.h>

namespace Concurrency {

#define CXXAMP_NOCACHE (1)

struct mm_info
{
#if CXXAMP_NOCACHE
    void *data;
    bool free;
#else
    size_t count;
    void *host;
    void *device;
    void *dirty;
    bool discard;
#endif

#if CXXAMP_NOCACHE
    mm_info(int count)
        : data(aligned_alloc(0x1000, count)), free(true) { getAllocator()->init(data, count); }
    mm_info(int count, void *src)
        : data(src), free(false) { getAllocator()->init(data, count); }
#else
    mm_info(int count)
        : count(count), host(aligned_alloc(0x1000, count)), device(host),
        dirty(host), discard(false) { getAllocator()->init(device, count); }
    mm_info(int count, void *src)
        : count(count), host(src), device(aligned_alloc(0x1000, count)),
        dirty(host), discard(false) { getAllocator()->init(device, count); }
#endif

#if CXXAMP_NOCACHE
    void synchronize() { getAllocator()->sync(data); }
    void refresh() {}
    void* get() { return data; }
    void copy(void *dst) { getAllocator()->copy(data, dst); }
    void disc() { getAllocator()->discard(data); }
    void serialize(Serialize& s) {
      getAllocator()->append(s.getKernel(), s.getAndIncCurrentIndex(), data);
    }
    ~mm_info() {
      synchronize();
      getAllocator()->free(data);
      if (free)
        ::operator delete(data);
    }
#else
    void synchronize() {
        if (dirty != host) {
            memmove(host, device, count);
            dirty = host;
        }
    }
    void refresh() {
        if (device != host)
            memmove(device, host, count);
    }
    void* get() { return dirty; }
    void disc() {
        if (dirty != host)
            dirty = host;
        discard = true;
    }
    void serialize(Serialize& s) {
        if (dirty == host && device != host) {
            if (!discard)
                refresh();
            dirty = device;
        }
        discard = false;
        getAllocator()->append(s.getKernel(), s.getAndIncCurrentIndex(), device);
    }
    ~mm_info() {
        getAllocator()->free(device);
        if (host != device) {
            if (!discard)
                synchronize();
            ::operator delete(device);
        }
    }
#endif
};

// Dummy interface that looks somewhat like std::shared_ptr<T>
template <typename T>
class _data {
public:
    _data() = delete;
    _data(int count) {}
    _data(const _data& d) restrict(cpu, amp)
        : p_(d.p_) {}
    template <typename U>
        _data(const _data<U>& d) restrict(cpu, amp)
        : p_(reinterpret_cast<T *>(d.get())) {}
    __attribute__((annotate("user_deserialize")))
        explicit _data(__global T* t) restrict(cpu, amp) { p_ = t; }
    __global T* get(void) const restrict(cpu, amp) { return p_; }
private:
    __global T* p_;
};

template <typename T>
class _data_host {
    std::shared_ptr<mm_info> mm;
    template <typename U> friend class _data_host;
public:
    _data_host(int count)
        : mm(std::make_shared<mm_info>(count * sizeof(T))) {}
    _data_host(int count, T* src)
        : mm(std::make_shared<mm_info>(count * sizeof(T), src)) {}
    _data_host(const _data_host& other)
        : mm(other.mm) {}
    template <typename U>
        _data_host(const _data_host<U>& other) : mm(other.mm) {}

    T *get() const { return (T *)mm->get(); }
    void synchronize() const { mm->synchronize(); }
    void discard() const { mm->disc(); }
    void refresh() const { mm->refresh(); }
    void copy(void *dst) const { mm->copy(dst); }

    __attribute__((annotate("serialize")))
        void __cxxamp_serialize(Serialize& s) const {
            mm->serialize(s);
        }
    __attribute__((annotate("user_deserialize")))
        explicit _data_host(__global T* t);
};

} // namespace Concurrency
#endif
