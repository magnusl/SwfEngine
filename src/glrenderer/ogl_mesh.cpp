#include "stdafx.h"
#include "ogl_shape.h"
#include "ogl_mesh.h"
#include <GL\glew.h>

using namespace std;

namespace ogl
{

/**
 * Creates a OpenGL mesh from a gfx::Mesh.
 */
std::shared_ptr<Mesh> create_ogl_mesh(const CPUMesh & gfxMesh)
{
	shared_ptr<ogl::Mesh> glMesh = make_shared<ogl::Mesh>();
	if (gfxMesh.VertexData.empty() || gfxMesh.Indicies.empty()) {
		return shared_ptr<ogl::Mesh>();
	}
	glMesh->Verticies = std::make_shared<ogl::VertexBuffer>(&gfxMesh.VertexData[0],
			gfxMesh.VertexData.size() * sizeof(Point), GL_STATIC_DRAW);

	glMesh->Indicies = make_shared<IndexBuffer>(&gfxMesh.Indicies[0],
			gfxMesh.Indicies.size() * sizeof(Triangle));

	if (!gfxMesh.TextureCoords.empty()) {
		glMesh->TexCoords = make_shared<VertexBuffer>(&gfxMesh.TextureCoords[0],
			gfxMesh.TextureCoords.size() * sizeof(Point), GL_STATIC_DRAW);
	}

	glMesh->Material		= gfxMesh.Material;
	glMesh->NumIndicies	    = gfxMesh.Indicies.size() * 3;
	//glMesh->ShapeBounds	= gfxMesh.Bounds;

	switch(gfxMesh.Material.MaterialType)
	{
	case Material::MATERIAL_LINEAR_GRADIENT:
	case Material::MATERIAL_RADIAL_GRADIENT:
		{
        #if 0
			uint8_t texture[1024];
			generateTexture(glMesh->m_Material.Gradient, texture, 1024);
			glMesh->m_Texture = std::make_shared<CGLTexture1D>(CGLTexture::eRGBA_TEXTURE, texture, 256);
			break;
        #endif
		}
	}
	return glMesh;
}

void create_ogl_shape(std::shared_ptr<CPUMeshes> meshes,
    std::shared_ptr<Shape> shape)
{
    if (!meshes->Meshes.empty())
    {
        for(std::list<CPUMesh>::const_iterator it = meshes->Meshes.begin();
            it != meshes->Meshes.end();
            it++)
        {
            shape->ogl_meshes.push_back(create_ogl_mesh(*it));
        } 
    }
}

} // namespace ogl