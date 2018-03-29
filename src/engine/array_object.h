#ifndef _ARRAY_OBJECT_H_
#define _ARRAY_OBJECT_H_

#include "swf_object.h"
#include "swf_array.h"

namespace swf_redux
{

/**
 * ActionScript 3 array object.
 */
struct ArrayObject
{
    ArrayObject(IAllocator & allocator) :
        _object(allocator),
        _array_value(allocator)
    {
    }

    Object  _object;

    /// member specific fields.
    Array<Object *> _array_value;
};

}

#endif