#ifndef OGL_DRAWABLE_H
#define OGL_DRAWABLE_H

#include <Eigen\Dense>
#include "ogl_rect.h"
#include "ogl_mesh.h"
#include "ogl_shape.h"

namespace ogl
{

struct Drawable
{
	enum {
		eMESH,
		eTEXT,
		eRECT,
		eCLIP_MASK,
		eCLIP_STOP
	} Type;

	struct {
		//ColorTransform			colorTransform;
		std::shared_ptr<Mesh>	    mesh;
		Eigen::Matrix3f				transformation;
		float						alpha;
		bool						hasColorTransformation;
	} mesh;

	struct {
		int					FontHandle;
		size_t				String;
		float				TextSize;
		Eigen::Matrix3f		Transformation;
	} text;

	struct {
		std::vector<std::shared_ptr<ogl::Mesh> >	Meshes;
		Eigen::Matrix3f			                    Transformation;
	} clipping;

	Rect<float> Bounds;
};

}

#endif
