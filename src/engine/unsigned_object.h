#ifndef _UNSIGNED_OBJECT_H_
#define _UNSIGNED_OBJECT_H_

#include "swf_object.h"
#include <stdint.h>

namespace swf_redux
{

/**
 * Unsigned integer object.
 */
struct UnsignedObject
{
    UnsignedObject(IAllocator & allocator) : 
        _object(allocator),
        _unsigned_value(0)
    {
    }

    Object      _object;

    /// class specific members
    uint32_t    _unsigned_value;
};

} // namespace swf_redux

#endif