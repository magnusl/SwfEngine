#include "swf_object.h"
#include <iostream>
#include "swf_allocator.h"
#include "array_object.h"
#include "sysinfo.h"

#ifdef WIN32
    #include "mmap_source.h"
#else
    #include "mmap_posix.h"
#endif

#include "swf_decode.h"
#include "swf_parser.h"
#include <memory>

using namespace std;

int main()
{
    try {
#ifdef WIN32
        shared_ptr<swf_redux::io::MMapSource> source = make_shared<swf_redux::io::MMapSource>(L"e:\\dog.swf");
#else
        shared_ptr<swf_redux::io::MMapPosix> source = make_shared<swf_redux::io::MMapPosix>("e:\\dog.swf");
#endif
        swf_redux::swf::SWFParser parser;
        if (!parser.parse(source)) {
            cerr << "Failed to parse source." << endl;
        }
        cout << "Parsed SWF." << endl;
    } catch(...) {
        cerr << "Caught exception." << endl;
        return -1;
    } 
    return 0;
}