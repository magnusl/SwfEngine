#ifndef SWF_SHAPE_H
#define SWF_SHAPE_H

#include <vector>
#include <stdint.h>
#include <memory>
#include <list>

#include "swf_types.h"

namespace swf_redux
{

namespace gfx
{

template<class T>
struct Point
{
    Point(T _x, T _y) : x(_x), y(_y)
    {
    }

    Point()
    {
        // empty
    }

    Point<T> operator+(const Point<T> & rhs)
    {
        return Point<T>(x + rhs.x, y + rhs.y);
    }

    Point<T> operator-(const Point<T> & rhs)
    {
        return Point<T>(x - rhs.x, y - rhs.y);
    }

    Point<T> operator*(float rhs)
    {
        return Point<T>(x * rhs, y * rhs);
    }


    inline bool operator==(const Point<T> & rhs) const
    {
        return (rhs.x == x) && (rhs.y == y);
    }

    T x, y;
};

/// 32-bit RGBA color.
///
struct Color
{
	uint8_t r,g,b,a;
};

struct FillStyle 
{
	enum {
		NoFill,			// Path does not have a fill
		SolidFill,		// Path is filled with a solid RGBA color
		BitmapFill,		// Path is filled with a bitmap
		LinearGradient,	// Path is filled with a linear gradient
		RadialGradient
	} FillType;

	union {
		uint32_t	bitmap_id;	// Id for bitmap fill
		Color		color;		// Color for solid fill
		struct {
			uint8_t	        num_control_points; // the number of control points
			struct {
				Color		color;
				uint8_t		ratio;
			} points[16];
		} gradient;
	} u;
};


struct LineStyle
{
    Color       color;
    uint16_t    line_width;
};

/// A straight or filled edge
///
struct Edge 
{
    Point<int32_t>  start_pos, control_pos, stop_pos;
    int16_t         fill0;
    int16_t         fill1;
    int16_t         line;          
    uint8_t         is_straight : 1;
};

/// A open or filled path
///
struct Path 
{
    std::list<Edge>         edges;
    std::vector<int16_t>    inner_paths;
    LineStyle               line_style;
    FillStyle               fill_stype;

    uint8_t                 is_closed : 1;
    uint8_t                 is_ccw    : 1;
};

struct Shape
{
    std::vector<Path>       paths;
    std::vector<uint16_t>   outer_paths;
    int16_t                 scale_factor;
    bool                    is_glyph;
    void *                  user_data;  // used by the renderer
};

std::shared_ptr<gfx::Shape> create_shape(const swf::DefinedShape4 & flash_shape);

}

}

#endif