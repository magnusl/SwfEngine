#include "threadpool.h"
#include "swf_semaphore.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <queue>
#include <vector>
#include <process.h>
#include <iostream>

using namespace std;

namespace swf_redux
{

namespace os
{

class ThreadPool::PoolImplementation
{
public:
    PoolImplementation(size_t num_threads);
    ~PoolImplementation();

    static DWORD WINAPI ThreadPoolThread(PVOID);
    
    vector<HANDLE>              _threads;
    queue<shared_ptr<IJob> >    _job_queue;
    CRITICAL_SECTION            _lock;
    CONDITION_VARIABLE          _buffer_not_empty;
    CONDITION_VARIABLE          _jobs_finished;
    size_t                      _num_jobs;
    bool                        _error_occurred;
    bool                        _aborted;
};

ThreadPool::PoolImplementation::PoolImplementation(size_t num_threads) :
_error_occurred(false),
_aborted(false),
_num_jobs(0)
{
    InitializeCriticalSection(&_lock);
    InitializeConditionVariable(&_buffer_not_empty);
    InitializeConditionVariable(&_jobs_finished);
    _threads.resize(num_threads);

    for(size_t i = 0; i < num_threads; ++i) {
        // create threads, as suspended since we don't want them to start
        // while we are still constructing the object.
        _threads[i] = CreateThread(NULL, 0, ThreadPool::PoolImplementation::ThreadPoolThread, this, 0, NULL);
        if (!_threads[i]) {
            // failure, release resources.
            for(size_t thread = 0; thread < i; ++thread) {
                CloseHandle(_threads[thread]);
                _threads[thread] = INVALID_HANDLE_VALUE;
            }
            DeleteCriticalSection(&_lock);
            // now throw a exception
            throw std::runtime_error("Failed to create thread for thread pool.");
        }
    }
}

ThreadPool::PoolImplementation::~PoolImplementation()
{
    for(size_t thread = 0; thread < _threads.size(); ++thread) {
        CloseHandle(_threads[thread]);
        _threads[thread] = INVALID_HANDLE_VALUE;
    }
    DeleteCriticalSection(&_lock);
}

/// Thread function used by the thread pool
///
DWORD WINAPI ThreadPool::PoolImplementation::ThreadPoolThread (PVOID p)
{
    ThreadPool::PoolImplementation * pool = (ThreadPool::PoolImplementation *) p;
    while(pool)
    {
        // acquire the lock
        EnterCriticalSection(&pool->_lock);

        if (pool->_aborted) {
            LeaveCriticalSection(&pool->_lock);
            break;
        }

        if (pool->_job_queue.empty()) {
            // no current jobs, so just sleep
            SleepConditionVariableCS(&pool->_buffer_not_empty, &pool->_lock, INFINITE); 
        }

        if (pool->_aborted) {
            LeaveCriticalSection(&pool->_lock);
            break;
        }

        if (pool->_job_queue.empty()) {
            LeaveCriticalSection(&pool->_lock);
            continue;
        }
        // pop a job of the queue
        shared_ptr<IJob> job = pool->_job_queue.front();
        pool->_job_queue.pop();
        // increment the number of active jobs
        ++pool->_num_jobs;
        // and unlock the critical section
        LeaveCriticalSection(&pool->_lock);

        // execute the job
        bool res = job->execute();
        // update state
        EnterCriticalSection(&pool->_lock);
        pool->_error_occurred = (res == false) ? true : pool->_error_occurred;
        --pool->_num_jobs;
        LeaveCriticalSection(&pool->_lock);

        WakeConditionVariable (&pool->_jobs_finished);
    }
    return 0;
}

ThreadPool::ThreadPool(size_t num_threads)
{
    _impl = new (std::nothrow) PoolImplementation(num_threads);
}

ThreadPool::~ThreadPool()
{
    // make sure that any created threads are stopped
    abort();
    delete _impl;
}

bool ThreadPool::submit_job(std::shared_ptr<IJob> job)
{
    // only add the job if the thread pool isn't in a aborted state.
    EnterCriticalSection(&_impl->_lock);
    bool status = _impl->_aborted ? false : true;
    if (status) {
        _impl->_job_queue.push(job);
    }
    LeaveCriticalSection(&_impl->_lock);
    WakeConditionVariable (&_impl->_buffer_not_empty);
    return status;
}

void ThreadPool::abort()
{
    EnterCriticalSection(&_impl->_lock);
    _impl->_aborted = true;
    LeaveCriticalSection(&_impl->_lock);

    while(1) {
        // wake all the threads
        WakeAllConditionVariable(&_impl->_buffer_not_empty);
        // now wait for the threads to terminate
        if (WaitForMultipleObjects(_impl->_threads.size(), &_impl->_threads[0], TRUE, INFINITE) == WAIT_OBJECT_0 ) {
            return;
        }
    }
}

void ThreadPool::wait()
{
    EnterCriticalSection(&_impl->_lock);
    while(1)
    {
        if (!_impl->_num_jobs && _impl->_job_queue.empty()) {
            // no active jobs and no jobs in the pipe.
            LeaveCriticalSection(&_impl->_lock);
            break;
        }
        SleepConditionVariableCS (&_impl->_jobs_finished, &_impl->_lock, INFINITE);
    }
}

bool ThreadPool::error_encountered()
{
    if (_impl) {
        EnterCriticalSection(&_impl->_lock);
        bool err = _impl->_error_occurred;
        LeaveCriticalSection(&_impl->_lock);
        return err;
    } else {
        return true;
    }
}

} // namespace os

} // namespace swf_redux