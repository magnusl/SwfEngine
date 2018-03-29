#ifndef OGL_RENDERER_H
#define OGL_RENDERER_H

#include <swf_renderer.h>

namespace ogl
{

/// OpenGL renderer implementation
///
class Renderer : swf_redux::renderer::RenderInterface
{
public:
    Renderer();
    ~Renderer();

    virtual bool initialize(size_t width, size_t height);
    virtual void draw(std::shared_ptr<swf_redux::gfx::Shape> shape, 
        const Eigen::Matrix3f & transform, float alpha);
    virtual void flush(void);
};

}

#endif