#ifndef SWF_RUNTIME_H
#define SWF_RUNTIME_H

#include "swf_object.h"
#include "class_object.h"

namespace swf_redux
{

///
///
class Runtime
{
public:
    /// returns the class object for a specific class
    ClassObject * get_class(ObjectType_t type);
};

}

#endif