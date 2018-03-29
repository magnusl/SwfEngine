#ifndef _BOOLEAN_OBJECT_H_
#define _BOOLEAN_OBJECT_H_

#include "swf_object.h"

namespace swf_redux
{

/**
 * Represents the ActionScript 3 "Boolean" object
 */
struct BooleanObject
{
    BooleanObject(IAllocator & allocator) :
        _object(allocator),
        _boolean_value(false)
    {
    }

    Object  _object;

    /// class specific members
    bool    _boolean_value;
};

}

#endif