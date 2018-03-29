#ifndef SYSINFO_HH
#define SYSINFO_HH

namespace swf_redux
{

namespace os
{

#if defined(_M_X86) || defined(_M_X64) || defined(__x86_64__)
struct x86_cpu_info
{
    uint32_t    sse    : 1;
    uint32_t    sse2   : 1;
    uint32_t    sse3   : 1;
    uint32_t    ssse3  : 1;
    uint32_t    sse4_1 : 1;
    uint32_t    sse4_2 : 1;
    uint32_t    avmx   : 1;
};

/// Retrive CPU specific information.
///
void get_cpu_information(x86_cpu_info &);

#endif

/// Returns the number of logical processors in the system
///
size_t get_logical_processor_count();

///
/// Returns true if the current processor supports 
}

}

#endif