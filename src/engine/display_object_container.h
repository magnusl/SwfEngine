#ifndef DISPLAY_OBJECT_CONTAINER_H
#define DISPLAY_OBJECT_CONTAINER_H

#include "display_object.h"
#include "swf_array.h"
#include "interactive_object.h"
#include "swf_renderer.h"

namespace swf_redux
{
namespace flash
{
namespace display
{

struct ContainerElement
{
    DisplayObject * _object;
    uint16_t        _depth;
};

/// flash.display.DisplayObjectContainer
///
struct DisplayObjectContainer
{
    // keep the same layout as InteractiveObject
    InteractiveObject _interactive_object;

    /// container children
    Array<ContainerElement> _children;
};

/// adds a child object to a display object container.
void add_child(DisplayObject *);

/// adds a child at a specific depth in the conntainer.
void add_child_at(DisplayObject *, uint16_t);

/// Draws a display object container.
void draw_display_container(flash::display::DisplayObject *, renderer::RenderInterface &);

}
}
}

#endif