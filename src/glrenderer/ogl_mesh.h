#ifndef OGL_MESH_H
#define OGL_MESH_H

#include <vector>
#include <cstdint>
#include <list>
#include "swf_shape.h"

namespace ogl
{

struct Point
{
    Point();
    Point(float _x, float _y) : x(_x), y(_y)
    {
    }

    Point operator * (float rhs) const
    {
        return Point(x * rhs, y * rhs);
    }

    float x, y;
};

struct Triangle
{
    Triangle()
    {
    }

    Triangle(uint32_t _p1, uint32_t _p2, uint32_t _p3) :
         p1(_p1),
         p2(_p2),
         p3(_p3)
    {
    }

    uint32_t p1, p2, p3;
};

template<class T>
class Bezier
{
public:
	Bezier(const swf_redux::gfx::Point<T> & p0, 
        const swf_redux::gfx::Point<T> & control,
        const swf_redux::gfx::Point<T> & p1) :
		m_p0(p0), 
        m_control(control),
        m_p1(p1)
	{
	}

	swf_redux::gfx::Point<T> Get(float t)
	{
		float dt = 1.0f - t;
		return (m_p0 * dt * dt) + (m_control*2.0f*dt*t) + (m_p1 * (t*t));
	}

protected:
	swf_redux::gfx::Point<T> m_p0, m_control, m_p1;
};

struct GradientPoint
{
	float	Color[4];
	uint8_t	Ratio;

	bool operator ==(const GradientPoint & a_Point) const {
		if (Ratio != a_Point.Ratio) {
			return false;
		}
		for(size_t i = 0; i < 4; ++i) {
			if (Color[i] != a_Point.Color[i]) {
				return false;
			}
		}
		return true;
	}
};

struct Gradient 
{
	GradientPoint   Points[16];
	uint8_t			Count;

	bool operator ==(const Gradient & a_Gradient) const {
		if (Count == a_Gradient.Count) {
			for(size_t i = 0; i < Count; ++i) {
				if (!(Points[i] == a_Gradient.Points[i])) {
					return false;
				}
			}
			return true;
		} else {
			return false;
		}
	}
};

struct Material 
{
	enum {
		MATERIAL_SOLID_COLOR,			/**< Mesh is rendered with a solid color */
		MATERIAL_TEXTURE,				/**< The mesh is textured */
		MATERIAL_LINEAR_GRADIENT,		/**< Linear gradient */
		MATERIAL_RADIAL_GRADIENT
	} MaterialType;

	union {
		float			Color[4];
		Gradient		Gradient;
	};

	bool operator ==(const Material & a_Material) const {
		if (MaterialType == a_Material.MaterialType) {
			switch(MaterialType) {
			case MATERIAL_SOLID_COLOR:
				for(size_t i = 0; i < 4; ++i) {
					if (Color[i] != a_Material.Color[i]) {
						return false;
					}
				}
				return true;
			case MATERIAL_TEXTURE:
				return false;
			case MATERIAL_LINEAR_GRADIENT:
			case MATERIAL_RADIAL_GRADIENT:
				return Gradient == a_Material.Gradient;
			default:
				return false;
			}
		} else {
			return false;
		}
	}
};	


struct CPUMesh
{
    std::vector<Point>      VertexData;
    std::vector<Point>      TextureCoords;
    std::vector<Triangle>   Indicies;
    Material                Material;
};

struct CPUMeshes
{
    std::list<CPUMesh>      Meshes;
};

}

#endif