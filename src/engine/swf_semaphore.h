#ifndef SWF_SEMAPHORE_HH
#define SWF_SEMAPHORE_HH

namespace swf_redux
{

namespace os
{

class Semaphore
{
public:
    Semaphore(int initial_count = 0);

    void wait();
    void signal();
    bool timed_wait(int timeout);
};

}

}

#endif