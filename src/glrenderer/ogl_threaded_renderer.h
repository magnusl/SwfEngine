#ifndef OGL_THREADED_RENDERER_H
#define OGL_THREADED_RENDERER_H

#include <swf_renderer.h>
#include <threadpool.h>
#include "ogl_shape.h"
#include <list>

namespace ogl
{

/// Threaded OpenGL renderer
///
class ThreadedRenderer : swf_redux::renderer::RenderInterface
{
public:
    ThreadedRenderer(size_t num_threads);
    ~ThreadedRenderer();

    virtual bool initialize(size_t width, size_t height);
    virtual void draw(std::shared_ptr<swf_redux::gfx::Shape> shape, const Eigen::Matrix3f & transform, float alpha);
    virtual void flush(void);

protected: /// types

    class Impl;
    Impl * _impl;
};

}

#endif