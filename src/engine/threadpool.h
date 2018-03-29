#ifndef THREADPOOL_HH
#define THREADPOOL_HH

#include <memory>

namespace swf_redux
{

namespace os
{

/// A job that can be scheduled for execution
///
class IJob
{
public:
    virtual ~IJob() { /* empty */ }
    virtual bool execute(void) = 0;
};

/// A thread pool executes jobs
///
class ThreadPool
{
public:
    ThreadPool(size_t num_threads);
    virtual ~ThreadPool();

    /// submits a job
    bool submit_job(std::shared_ptr<IJob>);
    /// aborts any running jobs
    void abort();
    /// waits for all the submitted jobs to be complete.
    void wait();
    /// indicates if a error has occured
    bool error_encountered();

protected:
    class PoolImplementation;

    PoolImplementation * _impl;

};

} 

}

#endif