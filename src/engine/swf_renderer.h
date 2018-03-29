#ifndef SWF_RENDERER_H
#define SWF_RENDERER_H

#include <memory>
#include "swf_shape.h"
#include <Eigen/Dense>

namespace swf_redux
{

namespace renderer
{

/// Defines the interface that must be implemented by renderers
///
class RenderInterface
{
public:
    /// virtual destructor
    virtual ~RenderInterface() { /* empty */ }

    /// Initializes the renderer
    virtual bool initialize(size_t width, size_t height)    = 0;

    /// Draws a shape using a specific transformation
    virtual void draw(std::shared_ptr<gfx::Shape> shape, 
        const Eigen::Matrix3f & transform, float alpha)     = 0;

    virtual void draw(const gfx::Shape * shape, 
        const Eigen::Matrix3f & transform, float alpha)     = 0;
    
    /// Flushes the drawing pipeline. Nothing is guaranteed to
    /// have been drawn before this function is called. 
    virtual void flush(void)                                = 0;
};

}

}

#endif