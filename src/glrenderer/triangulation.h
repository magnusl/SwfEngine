#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <swf_shape.h>
#include "ogl_mesh.h"

namespace ogl
{
/// Triangulates a SWF shape into one or more triangle meshes
///
bool triangulate_shape(const swf_redux::gfx::Shape & shape,
    CPUMeshes & meshes, float ErrorTolerance);
}

#endif