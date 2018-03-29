#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "swf_shape.h"

namespace swf_redux
{
namespace gfx
{

/// Check if a horizontal line starting from a_Point intersects with the straight edge.
///
bool intersects_straight(const Point<int32_t> & point,
	const Edge & edge,
	Point<int32_t> & intersection_point);

/// Counts how many times a horizontal line starting from a_Point 
/// intersects a quadratic bezier curve.
///
int count_intersections(const Point<int32_t> & point,
	const Point<int32_t> & start, const Point<int32_t> & control, const Point<int32_t> & stop);

/// Counts how many times a horizontal line starting at the point intersects a edge.
///
size_t count_intersections(const Point<int32_t> & point, const Edge & edge);

///  Checks if the point is inside the closed path defined by the supplied edges.
/// Uses the odd-even rule to determine if a shape is inside or outside.
/// param [in] a_Point	The point to check if it's inside.
/// param [in] a_Edges	The edges defining a closed shape.
///
bool test_point(const Point<int32_t> & point, const std::list<Edge> & edges);

/// Checks that the shape defined by the a_OuterEdges fully enclose the shape defined
/// by the a_InnerEdges. This is a very simple test that only tests that the Start and Stop points
/// are inside. This should be sufficient to determine if two SWF shapes overlap, since the Flash toolset 
/// doesn't generate overlapping paths.
///
bool is_path_inside(const std::list<Edge> & outerEdges, 
	const std::list<Edge> & innerEdges);

/// Tests if a point (specified in Twips) is inside the shape.
///
bool test_twips_point(const Shape & a_Shape, Point<int32_t> & a_Point);

bool test_point_with_shape(const Shape & a_Shape, Point<float> & a_Point);

}

}


#endif