#ifndef TRIANGULATION_JOB_H
#define TRIANGULATION_JOB_H

#include <threadpool.h>
#include <swf_shape.h>
#include <Eigen\Dense>
#include <memory>
#include "ogl_shape.h"

namespace ogl
{

/// Job that triangulates a shape to a set of meshes
///
class TriangulationJob : public swf_redux::os::IJob
{
public:
    /// constructor
    TriangulationJob(std::shared_ptr<Shape> cached,
        std::shared_ptr<swf_redux::gfx::Shape> shape,
        const Eigen::Matrix3f & transform);
    
    /// IJob interface
    virtual bool execute(void);

private:
    std::shared_ptr<Shape>                 _cached;
    std::shared_ptr<swf_redux::gfx::Shape> _swf_shape;
    Eigen::Matrix3f                        _transformation;
};

}

#endif