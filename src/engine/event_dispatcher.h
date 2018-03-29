#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

#include "swf_object.h"
#include "swf_array.h"

namespace swf_redux
{
namespace flash
{
namespace events
{

struct Entry {
    Object *    handler;
    uint16_t    _name;
    bool        _use_capture;
};

/// flash.events.EventDispatcher
///
struct EventDispatcher
{
    EventDispatcher(IAllocator & allocator) : _object(allocator), _listeners(allocator)
    {
        // empty
    }

    Object _object;

    /// EventDispatcher
    Array<Entry> _listeners;
};

}

}

}

#endif