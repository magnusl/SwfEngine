#ifndef _SWF_OBJECT_H_
#define _SWF_OBJECT_H_

#include <stdint.h>
#include "swf_array.h"
#include "swf_allocator.h"

namespace swf_redux
{

struct ClassObject;

/// a ActionScript 3 object.
struct Object
{
    Object(IAllocator & allocator) : 
        _dynamic_property_names(allocator),
        _dynamic_property_values(allocator),
        _fixed_trait_values(allocator)
    {
    }

    uint32_t    _object_type     : 10;
    uint32_t    _reference_count : 10;

    // constructor object.
    Object *    _constructor;

    swf_redux::Array<uint16_t> _dynamic_property_names;
    swf_redux::Array<Object *> _dynamic_property_values;
    swf_redux::Array<Object *> _fixed_trait_values;
};

typedef uint16_t    ObjectType_t;

} // namespace swf_redux

#endif