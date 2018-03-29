#include "display_object_container.h"
#include "display_operations.h"

namespace swf_redux
{
namespace flash
{
namespace display
{

/// Draws a display object container.
void draw_display_container(flash::display::DisplayObject * obj, 
    renderer::RenderInterface & renderer)
{
    DisplayObjectContainer * container = (DisplayObjectContainer *) obj;
    
    size_t num_children = array::size(container->_children);
    if (num_children > 0) {
        for(int i = (int) num_children - 1; i >= 0; --i){
            DisplayObject * child = container->_children[i]._object;
            if (child && child->_operations) {
                child->_operations->DrawObject(child, renderer);
            }
        }
    }
}

} // namespace display

} // namespace flash

} // namespace swf_redux