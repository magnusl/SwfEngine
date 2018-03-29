#ifndef SHAPE_H
#define SHAPE_H

#include "display_object.h"
#include "swf_shape.h"
#include "swf_renderer.h"

namespace swf_redux
{

namespace flash
{

namespace display
{

/// flash.display.Shape
///
struct Shape
{
    DisplayObject   _display_object;

    /// SWF Shape
    swf_redux::gfx::Shape * _shape;
};

/// Draws a shape object
void draw_shape(flash::display::DisplayObject *, renderer::RenderInterface &);

/// Performs hit-collision againts the shape
bool hittest_shape(flash::display::DisplayObject *, const gfx::Point<float> &, bool);

} // namespace display

} // namespace flash

} // namespace swf_redux

#endif