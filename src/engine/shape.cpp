#include "shape.h"

namespace swf_redux
{

namespace flash
{

namespace display
{

/////
// Internal implementation of the shape class object.
struct ShapeClass_Impl
{
	ShapeClass_Impl(PageAllocator & page_allocator) : _allocator(page_allocator)
	{
		// empty.
	}
	// allocates object of a specific size.
	FixedSizeAllocator<sizeof(Shape)> _allocator;
};

/// Draws a shape object
void draw_shape(flash::display::DisplayObject * object, 
    renderer::RenderInterface & renderer)
{
    Shape * shape = (Shape *) object;
    if (shape) {
        renderer.draw(shape->_shape, object->_local_transform, object->_alpha);
    }
}

/////
// Creates a shape object.
Object * construct_shape(ClassObject * class_object)
{
	ShapeClass_Impl * impl = (ShapeClass_Impl *) class_object->_impl;
	if (!impl) {
		return nullptr;
	}
	// allocate memory for a shape instance.
	void * mem = impl->_allocator.allocate();
	if (!mem) {
		return nullptr;
	}
	return new (mem) Shape(dyn_allocator);
}

/////
// reclaims a previously allocated shape instance.
void reclaim_shape(ClassObject * class_object, Object * instance)
{
	ShapeClass_Impl * impl = (ShapeClass_Impl *) class_object->_impl;
	if (!impl) {
		return nullptr;
	}
	((Shape *)shape)->~Shape();
	// and now reclaim the actual memory
	impl->_allocator.deallocate(shape);
}

/////
// constructs a shape class instance.
ClassObject * construct_shape_class(PageAllocator & page_allocator)
{
	// create a new class object
	ClassObject * cobj = new ClassObject(allocator);

	// set the correct class operations.
	obj->_operations.construct 	= &construct_shape;
	obj->_operations.reclaim 	= &reclaim_shape;

	return cobj;
}

} // namespace display

} // namespace flash

} // namespace swf_redux