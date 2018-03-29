#include "swf_object.h"
#include <stdint.h>

namespace swf_redux
{

/// Object that represents a 32 bit signed integer
struct IntegerObject
{
    IntegerObject(IAllocator & allocator) : 
        _object(allocator),
        _integer_value(0)
    {
    }

    Object  _object;

    /// class specific members
    int32_t _integer_value;
};

}