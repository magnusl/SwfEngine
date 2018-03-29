#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace swf_redux
{

namespace os
{

/// Critical section
///
class CriticalSection
{
public:
    CriticalSection()
    {
        InitializeCriticalSection(&_cs);
    }

    inline void enter() { EnterCriticalSection(&_cs); }

    inline void leave() { LeaveCriticalSection(&_cs); }

private:
#ifdef WIN32
    CRITICAL_SECTION _cs;
#endif
};

class Lock
{
public:
    Lock(CriticalSection & cs) : _cs(cs)
    {
        _cs.enter();
    }

    ~Lock()
    {
        _cs.leave();
    }

protected:
    CriticalSection & _cs;
};

}

}

#endif