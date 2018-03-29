#include "stdafx.h"
#include "triangulation_job.h"
#include "triangulation.h"

namespace ogl
{

/// Constructor, initializes the job instance.
TriangulationJob::TriangulationJob(std::shared_ptr<Shape> cached,
    std::shared_ptr<swf_redux::gfx::Shape> shape,
    const Eigen::Matrix3f & transform) :
    _cached(cached),
    _swf_shape(shape),
    _transformation(transform)
{
    // empty
}

/// Performs the actual triangulation and creates the mesh
bool TriangulationJob::execute(void)
{
    // triangulate the shape
    return triangulate_shape(*_swf_shape, *_cached->cpu_meshes, _cached->error_tolerance);
}



}