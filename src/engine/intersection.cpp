#include "swf_shape.h"

namespace swf_redux
{

namespace gfx
{
    
/// Check if a horizontal line starting from a_Point intersects with the straight edge.
///
bool intersects_straight(const Point<int32_t> & a_Point,
	const Edge & a_Edge,
	Point<int32_t> & a_IntersectionPoint)
{
	int32_t ymin = std::min(a_Edge.start_pos.y, a_Edge.stop_pos.y), 
		ymax = std::max(a_Edge.start_pos.y, a_Edge.stop_pos.y);
	if (a_Point.y >= ymin && a_Point.y <= ymax) /**< overlaps Y */
	{
		if (a_Edge.start_pos.y == a_Edge.stop_pos.y) {
			int xmax = std::max(a_Edge.start_pos.x, a_Edge.stop_pos.x);
			/** TODO, calculate the distance */
			return xmax >= a_Point.x;
		} else {
			/** check where the line intersects the Y axis */
			int32_t dy = a_Edge.stop_pos.y - a_Edge.start_pos.y, dx = a_Edge.stop_pos.x - a_Edge.start_pos.x;
			float t				= float(a_Point.y - a_Edge.start_pos.y) / dy;
			float x				= float(a_Edge.start_pos.x) + dx * t;
			a_IntersectionPoint = Point<int32_t>(int32_t(x), a_Point.y);

			return (x >= a_Point.x);
		}
	}
	return false;
}

/// Counts how many times a horizontal line starting from a_Point 
/// intersects a quadratic bezier curve.
///
int count_intersections(const Point<int32_t> & a_Point,
	const Point<int32_t> & a_Start, const Point<int32_t> & a_Control, const Point<int32_t> & a_Stop)
{
	int		count = 0;
	int		a = a_Start.y - (2 * a_Control.y) + a_Stop.y;
	int		b = (2 * a_Control.y) - (2 * a_Start.y);
	int		c = a_Start.y - a_Point.y;

	if (a == 0) { /**< single solution */
		if (b != 0) {
			float t = -c / float(b);
			if ((t >= 0.0f) && (t <= 1.0f)) { /**< check if root is inside [0, 1] range */
				float dt = 1.0f - t;
				float x = dt*dt*float(a_Start.x) + (2 * dt * t * float(a_Control.x)) + (t * t * float(a_Stop.x));
				if (x >= a_Point.x) {
					return 1; /**< ray intersects curve */
				}
			}
		}
		return 0;
	}
	int fac = (b*b) - (4*a*c);
	if (fac < 0) { /**< can't take the square root of a negative number */
		return 0;
	}
	/** Calculate the roots of the quadratic equation */
	float root1 = (-b + sqrtf(float(fac))) / float(2*a);
	float root2 = (-b - sqrtf(float(fac))) / float(2*a);

	/** The roots are valid if the lie in the [0, 1] range */
	if (root1 >= 0.0f && root1 <= 1.0f) { /**< first root is valid */
		float dt = 1.0f - root1;
		float x = dt*dt*float(a_Start.x) + (2 * dt * root1 * float(a_Control.x)) + (root1 * root1 * float(a_Stop.x));
		if (x >= a_Point.x) {
			++count;
		}
	}
	if (root2 >= 0.0f && root2 <= 1.0f) { /**< second root is valid */
		float dt = 1.0f - root2;
		float x = dt*dt*float(a_Start.x) + (2 * dt * root2 * float(a_Control.x)) + (root2 * root2 * float(a_Stop.x));
		if (x >= a_Point.x) {
			++count;
		}
	}
	return count;
}

/// Counts how many times a horizontal line starting at the point intersects a edge.
///
size_t count_intersections(const Point<int32_t> & a_Point, const Edge & a_Edge)
{
	/** handle the corner case that the ray intersects the vertex between to edges,
		in this case just ignore the intersection with the stop edge */
	if ((a_Point.y == a_Edge.stop_pos.y) && (a_Point.x < a_Edge.stop_pos.x)) {
		return 0;
	}
	Point<int32_t> intersectionPoint;
	if (a_Edge.is_straight) {
		return intersects_straight(a_Point, a_Edge, intersectionPoint) ? 1 : 0;
	} else {
		return count_intersections(a_Point, 
			a_Edge.start_pos, a_Edge.control_pos, a_Edge.stop_pos);
	}
}

///  Checks if the point is inside the closed path defined by the supplied edges.
/// Uses the odd-even rule to determine if a shape is inside or outside.
/// param [in] a_Point	The point to check if it's inside.
/// param [in] a_Edges	The edges defining a closed shape.
///
bool test_point(const Point<int32_t> & a_Point, const std::list<Edge> & a_Edges)
{
	size_t count = 0;

    for(std::list<Edge>::const_iterator it = a_Edges.begin();
        it != a_Edges.end();
        it++)
    {
        count += count_intersections(a_Point, *it);
    }
	if (count & 1) { /**< lowest bit is set, so the count is odd */
		return true;
	} else {
		return false;
	}
}

/// Checks that the shape defined by the a_OuterEdges fully enclose the shape defined
/// by the a_InnerEdges. This is a very simple test that only tests that the Start and Stop points
/// are inside. This should be sufficient to determine if two SWF shapes overlap, since the Flash toolset 
/// doesn't generate overlapping paths.
///
bool is_path_inside(const std::list<Edge> & a_OuterEdges, 
	const std::list<Edge> & a_InnerEdges)
{
#if 0
	// for all the inner edges.
	for(size_t innerEdge = 0, numInner = a_InnerEdges.size(); 
		innerEdge < numInner;
		++innerEdge)
	{
		// check that the Start, Control and Stop point is inside the a_OuterEdges.
		if (!test_point(a_InnerEdges[innerEdge].start_pos, a_OuterEdges) ||
			!test_point(a_InnerEdges[innerEdge].stop_pos, a_OuterEdges)) 
		{
			return false;	
		}
	}
#endif
	return true;
}

/// Tests if a point (specified in Twips) is inside the shape.
///
bool test_twips_point(const Shape & a_Shape, Point<int32_t> & a_Point)
{
	if (a_Shape.outer_paths.empty()) { /**< Nothing to test with, cannot be inside */
		return false;
	}
	// A shape can have more than one actual shapes.
	const std::vector<uint16_t> & outer = a_Shape.outer_paths;
	for(size_t i = 0, num = outer.size(); i < num; ++i)
	{
		const Path & path = a_Shape.paths[outer[i]];
		if (path.is_closed) /**< This is a closed path */
		{
			if (test_point(a_Point, path.edges)) /**< inside this outer shape */
			{
				// TODO: Test with inner shapes.
				return true;
			}
		}
	}
	return false;
}

bool test_point_with_shape(const Shape & a_Shape, Point<float> & a_Point)
{
	Point<int32_t> twipsPoint((int32_t)a_Point.x * 20,
		(int32_t) (a_Point.y * 20));
	return test_twips_point(a_Shape, twipsPoint);
}

}

}