#ifndef DISPLAY_OBJECT_H
#define DISPLAY_OBJECT_H

#include "swf_object.h"
#include "event_dispatcher.h"
#include <Eigen/Dense>

namespace swf_redux
{

struct DisplayOperations;

namespace flash
{
namespace display
{

/// flash.display.DisplayObject
///
struct DisplayObject
{
    DisplayObject(IAllocator & allocator) : _dispatcher(allocator)
    {
        // empty
    }

    /// This must be the first member so that the layout is the same as EventDispatcher
    flash::events::EventDispatcher      _dispatcher;

    /// DisplayObject
    Eigen::Matrix3f                     _local_transform;
    DisplayOperations *                 _operations;
    DisplayObject *                     _parent;
    float                               _position_x, _position_y;
    float                               _scale_x, _scale_y;
    float                               _rotation;
    float                               _alpha;
    uint16_t                            _clip_depth;
    bool                                _dirty;
};

/// Get the display objects global transformation.
void get_global_transformation(DisplayObject *, Eigen::Matrix3f &);

/// Get the display objects local transformation.
const Eigen::Matrix3f & get_local_transformation(DisplayObject *);

}
}
}

#endif