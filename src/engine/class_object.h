#ifndef _CLASSOBJECT_H_
#define _CLASSOBJECT_H_

#include "swf_object.h"

namespace swf_redux
{

struct ClassObject;
/// Defines operations that can be performed on a class object such as
/// creating instances of it.
///
struct ClassOperations
{
    /// constructs an instance of the object.
    typedef Object * (*construct) (ClassObject *);
    /// reclaims a previously constructed class instance.
    typedef void (*reclaim) (ClassObject * class_object, Object * instance);
};

/**
 * ActionScript 3.0 class object
 */
struct ClassObject
{
    ClassObject(IAllocator & allocator) :
        _object(allocator)
    {
    }

    Object  _object;

    ClassOperations     _operations;
    /// prototype objects shared by instances of this class.
    Object *            _instance_prototype;
};

}

#endif