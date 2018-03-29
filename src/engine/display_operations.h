#ifndef DISPLAY_OPERATIONS_H
#define DISPLAY_OPERATIONS_H

#include "swf_renderer.h"
#include "display_object.h"
#include "swf_shape.h"

namespace swf_redux
{

/// Operations that can be performed on a display object.
///
struct DisplayOperations
{
    /// draws a object
    void (*DrawObject) (flash::display::DisplayObject *, renderer::RenderInterface &);
    /// hit test
    bool (*HitTest) (flash::display::DisplayObject *, const gfx::Point<float> &, bool);
};

}

#endif