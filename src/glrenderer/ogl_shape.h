#ifndef OGL_SHAPE_H
#define OGL_SHAPE_H

#include "ogl_mesh.h"
#include "index_buffer.h"
#include "vertex_buffer.h"
#include <memory>
#include <vector>

namespace ogl
{

struct Mesh
{
    std::shared_ptr<IndexBuffer>		Indicies;	// indicies into verticies and texcoords
	std::shared_ptr<VertexBuffer>	    Verticies;	// vertex data
	std::shared_ptr<VertexBuffer>	    TexCoords;	// texture coordinates
	Material					    	Material;
	size_t								NumIndicies;
};

struct Shape
{
    float                               error_tolerance;
	std::list<std::shared_ptr<Mesh> >	ogl_meshes;
    std::shared_ptr<CPUMeshes>          cpu_meshes;
};

void create_ogl_shape(std::shared_ptr<CPUMeshes> meshes, std::shared_ptr<Shape> shape);

} // namespace ogl;

#endif