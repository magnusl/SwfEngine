#include "stdafx.h"
#include "ogl_renderer.h"

namespace ogl
{

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

bool Renderer::initialize(size_t width, size_t height)
{
    return false;
}

void Renderer::draw(std::shared_ptr<swf_redux::gfx::Shape> shape, 
        const Eigen::Matrix3f & transform, float alpha)
{
}

void Renderer::flush(void)
{
}

}