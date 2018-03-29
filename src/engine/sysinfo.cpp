#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(__gnu_linux__)
#include <unistd.h>
#else
#error "Platform not supported"
#endif

namespace swf_redux
{

namespace os
{

#if defined(WIN32)

size_t get_logical_processor_count()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );
    return sysinfo.dwNumberOfProcessors;
}

#elif defined(__gnu_linux__)

size_t get_logical_processor_count()
{
    long res = sysconf (_SC_NPROCESSORS_CONF);
    // default to one if we get a error.
    return (res <= 0 ? 1 : ((size_t) res));
}

#endif
}

}