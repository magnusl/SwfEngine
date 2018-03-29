#include "swf_types.h"
#include "swf_shape.h"
#include "intersection.h"
#include <memory>
#include <map>
#include <list>
#include <iostream>

using namespace std;

namespace swf_redux
{

namespace gfx
{

/// Converts a flash fillstyle to a gfx fillstyle
///
static void convert_fill(const swf::FillStyle & flash_style,
    gfx::FillStyle & gfx_style)
{
    switch(flash_style.FillType)
    {
    case swf::FillStyle::eSOLID_RGBA:
        gfx_style.FillType  = FillStyle::SolidFill;
        gfx_style.u.color.r = flash_style.color.rgba.r;
        gfx_style.u.color.g = flash_style.color.rgba.g;
        gfx_style.u.color.b = flash_style.color.rgba.b;
        gfx_style.u.color.a = flash_style.color.rgba.a;
        break;
    case swf::FillStyle::eLINEAR_GRADIENT:
	case swf::FillStyle::eRADIAL_GRADIENT:
        if (flash_style.FillType == swf::FillStyle::eLINEAR_GRADIENT) {
			gfx_style.FillType = gfx::FillStyle::LinearGradient;
		} else {
			gfx_style.FillType = gfx::FillStyle::RadialGradient;
		}
		gfx_style.u.gradient.num_control_points	= flash_style.LinearGradient.numControlPoints;
		for(size_t i = 0; i < gfx_style.u.gradient.num_control_points; ++i) {
			const swf::Color_t & color = flash_style.LinearGradient.controlPoint[i].color;
			gfx_style.u.gradient.points[i].ratio = flash_style.LinearGradient.controlPoint[i].ratio;
			gfx_style.u.gradient.points[i].color.r = color.rgba.r;
			gfx_style.u.gradient.points[i].color.g = color.rgba.g;
			gfx_style.u.gradient.points[i].color.b = color.rgba.b;
			gfx_style.u.gradient.points[i].color.a = color.rgba.a;
		}
		break;
    default:
        break;
    }
}

static void convert_style(const swf::LineStyle & flash_style, gfx::LineStyle & linestyle)
{
	switch(flash_style.type) 
	{
	case swf::LineStyle::LINESTYLE1:
		linestyle.line_width   = flash_style.LineStyle1.width;
		linestyle.color.r     = flash_style.LineStyle1.color.rgba.r;
		linestyle.color.g     = flash_style.LineStyle1.color.rgba.g;
		linestyle.color.b     = flash_style.LineStyle1.color.rgba.b;
		linestyle.color.a     = flash_style.LineStyle1.color.rgba.a;
		break;
	case swf::LineStyle::LINESTYLE2:
		linestyle.line_width   = flash_style.Linestyle2.Width;
		linestyle.color.r     = flash_style.Linestyle2.Color.rgba.r;
		linestyle.color.g     = flash_style.Linestyle2.Color.rgba.g;
		linestyle.color.b     = flash_style.Linestyle2.Color.rgba.b;
		linestyle.color.a     = flash_style.Linestyle2.Color.rgba.a;
		break;
	}
}

static void records_to_edges(const std::vector<swf::ShapeRecord> & records,
    std::vector<Edge> & edges)
{
    // the initial position is at 0,0
    Point<int32_t> current_pos(0, 0);

    int32_t lineStyle = -1, fill0 = -1, fill1 = -1;
	for(size_t i = 0, numRecords = records.size(); i < numRecords; ++i) {
		const swf::ShapeRecord & record = records[i];
		switch(record.m_RecordType) {
		case swf::ShapeRecord::STYLE_CHANGE_RECORD:
			{
				if (record.u.StyleChanged.StateMoveTo) {
					current_pos = Point<int32_t>(
						record.u.StyleChanged.MoveDeltaX, record.u.StyleChanged.MoveDeltaY);
				}
				if (record.u.StyleChanged.StateFillStyle0) {
					fill0 = record.u.StyleChanged.StateFillStyle0;
				}
				if (record.u.StyleChanged.StateFillStyle0) {
					fill1 = record.u.StyleChanged.FillStyle1;
				}
				if (record.u.StyleChanged.StateLineStyle) {
					lineStyle = record.u.StyleChanged.LineStyle;
				}
				break;
			}
		case swf::ShapeRecord::STRAIGHT_EDGE_RECORD: /**< straight edge */
			{
				Edge edge;
				edge.is_straight	    = 1;/**< This is a straight edge */
				edge.start_pos			= current_pos;
				edge.stop_pos			= current_pos + Point<int32_t>(
					record.u.StraightEdge.DeltaX, record.u.StraightEdge.DeltaY);
				current_pos				= edge.stop_pos;
				edge.fill0			    = fill0;
				edge.fill1			    = fill1;
				edge.line			    = lineStyle;
				edges.push_back(edge);
			}
			break;
		case swf::ShapeRecord::CURVED_EDGE_RECORD: /** Quadratic Bezier curve */
			{
				Edge edge;
				edge.is_straight	= 0;/**< This is not a straight edge */
				edge.start_pos			= current_pos;
				edge.control_pos		= current_pos + Point<int32_t>(
					record.u.CurvedEdge.ControlDeltaX, record.u.CurvedEdge.ControlDeltaY);
				edge.stop_pos			= edge.control_pos + Point<int32_t>(
					record.u.CurvedEdge.AnchorDeltaX, record.u.CurvedEdge.AnchorDeltaY);
				current_pos				= edge.stop_pos;
				edge.fill0			    = fill0;
				edge.fill1			    = fill1;
				edge.line			    = lineStyle;
				edges.push_back(edge);
				break;
			}
		}
	}

}

inline Edge reverseEdge(const Edge & a_Edge)
{
	Edge edge;
	edge.control_pos	= a_Edge.control_pos;
	edge.start_pos		= a_Edge.stop_pos;
	edge.stop_pos		= a_Edge.start_pos;
	edge.is_straight    = a_Edge.is_straight;
	return edge;
}

inline bool isCCW(const std::list<Edge> & a_Edges)
{
    if (a_Edges.empty()) {
        return false;
    }

    int32_t sum = 0;
    const Point<int32_t> * prev = 0;
    for(std::list<Edge>::const_iterator it = a_Edges.begin();
        it != a_Edges.end();
        it++)
    {
        if (prev) {
            sum += (prev->x * it->start_pos.y - it->start_pos.x * prev->y);
        }
        prev = &it->start_pos;
    }

    const Point<int32_t> & first    = a_Edges.front().start_pos;
    const Point<int32_t> & last     = a_Edges.back().start_pos;

    sum += (last.x * first.y - first.x * last.y);
    return sum < 0;
}

static void checkOverlaps(gfx::Shape & a_Shape)
{
	size_t numPaths = a_Shape.paths.size();
	for(size_t current = 0; current < numPaths; ++current) { /**< for each path */
		bool isInsideAnother = false;
		for(size_t tested = 0; tested < numPaths; ++tested) { /**< test against all other paths */
			if (current != tested) { /**< test if the current one is inside the tested one */
				if (a_Shape.paths[tested].is_closed) {
					if (is_path_inside(a_Shape.paths[tested].edges, a_Shape.paths[current].edges)) {
						/** The current one is inside the tested one */
						a_Shape.paths[tested].inner_paths.push_back(current);
						isInsideAnother = true;
                        break;
					}
				}
			}
		}
		if (!isInsideAnother) { 
			/**< the current shape is not inside another one, so it's a outer shape */
			a_Shape.outer_paths.push_back(current);
		}
	}
}

void tesselate_path(const std::vector<Edge> & edges, 
				   const std::vector<LineStyle> & lineStyles,
				   const std::vector<FillStyle> & fillStyles,
				   Shape & shape)
{
	std::map<int16_t, std::list<Edge> > SortedEdges;
	for(size_t i = 0, num = edges.size(); i < num; ++i)
	{
		const Edge & edge = edges[i];
		if ((edge.fill1 > 0) && (edge.fill0 < 1)) { // We only have a right fillstyle.
			SortedEdges[edge.fill1].push_back(edge);
		} else if ((edge.fill1 < 1) && (edge.fill0 > 0)) {
			// We only have a left fillstyle, convert it to a edge with a right
			Edge reversed       = reverseEdge(edge);
			reversed.line		= edge.line;
			reversed.fill0		= -1;
			reversed.fill1		= edge.fill0;
			SortedEdges[reversed.fill1].push_back(reversed);
		} else if ((edge.fill0 > 0) && (edge.fill1 > 0)) {
			// We have two different fill styles, create two distinct edges,
			// first the right edge.
			Edge rightEdge      = edge;
			rightEdge.fill0	    = -1;
			rightEdge.fill1 	= edge.fill1;
			rightEdge.line	    = edge.line;
			SortedEdges[rightEdge.fill1].push_back(rightEdge);
			// now create the left edge.
			Edge leftEdge   = reverseEdge(edge);
			leftEdge.line	= edge.line;
			leftEdge.fill1  = edge.fill0;
			leftEdge.fill0	= -1;
			SortedEdges[leftEdge.fill1].push_back(leftEdge);
		} else { // No fill style.
			SortedEdges[0].push_back(edge);
		}
	}
    
    size_t num_cmps = 0;

	// Now find the connected edges in the different sorted slots
	for(map<int16_t, list<Edge> >::iterator it = SortedEdges.begin();
		it != SortedEdges.end();
		it++)
	{
    		list<Edge> & edgeList = it->second;
		while(!edgeList.empty()) 
		{
			Edge current = edgeList.front(); edgeList.pop_front();
			gfx::Path path;
			path.edges.push_back(current);	/**< add the starting edge */
			bool foundConnectedEdge;

			Point<int32_t> setStart(current.start_pos);
			Point<int32_t> setStop(current.stop_pos);

			path.is_closed = 0;
			do {
				foundConnectedEdge = false;
				for(list<Edge>::iterator testedEdge = edgeList.begin();
					testedEdge != edgeList.end();
					testedEdge++)
				{
                     ++num_cmps;
					if (testedEdge->stop_pos == setStart) {
                       
						/** prepend this edge */
						path.edges.insert(path.edges.begin(), *testedEdge);
						/** check if the edge closes the path */
						if (testedEdge->start_pos == setStop) {
							foundConnectedEdge = false;
							path.is_closed = 1;
						} else {
							foundConnectedEdge = true;
						}
						setStart = testedEdge->start_pos;
						edgeList.erase(testedEdge);
						break;
					} else if (testedEdge->start_pos == setStop) {
                        
						/** Append this edge */
						path.edges.push_back(*testedEdge);		/**< Add the connected edge */
						foundConnectedEdge = true;
						setStop = testedEdge->stop_pos;
						edgeList.erase(testedEdge);
						break;
					}
				}
			} while(foundConnectedEdge);
			/** Set the linestyle of the path */
			if (current.line < 1) {
				path.line_style.line_width = 0;
			} else {
				path.line_style = lineStyles[current.line - 1];
			}
			/** Set the fillstyle of the path */
			if (it->first < 1) {
				path.fill_stype.FillType = FillStyle::NoFill;
			} else if ((it->first - 1) < (int16_t) fillStyles.size()) {
				path.fill_stype = fillStyles[it->first-1];
			} else {
				throw std::runtime_error("FillStyle out of bounds.");
			}
			path.is_ccw = isCCW(path.edges) ? 1 : 0;
			shape.paths.push_back(path);
		}
	}
	checkOverlaps(shape);
}

shared_ptr<Shape> create_shape(const swf::DefinedShape4 & flash_shape)
{
    shared_ptr<Shape> shape = make_shared<Shape>();
    
    shape->is_glyph = false;
    
    size_t num_fills    = flash_shape.Shapes.FillStyles.size();
    size_t num_styles   = flash_shape.Shapes.LineStyles.size();

    vector<gfx::FillStyle> fillstyles(num_fills);
    vector<gfx::LineStyle> linestyles(num_styles);

    // convert the fill styles
    for(size_t i = 0; i < num_fills; ++i) {
        convert_fill(flash_shape.Shapes.FillStyles[i], fillstyles[i]);
    }
    // convert the line styles
    for(size_t i = 0; i < num_styles; ++i) {
        convert_style(flash_shape.Shapes.LineStyles[i], linestyles[i]);
    }
    // generate edges from shape records
    vector<Edge> shape_edges;
    records_to_edges(flash_shape.Shapes.Shapes, shape_edges);
    tesselate_path(shape_edges, linestyles, fillstyles, *shape);

	shape->scale_factor		= 20;
	//shape->Bounds.left	= flash_shape.ShapeBounds.Xmin;
	//shape->Bounds.right	= flash_shape.ShapeBounds.Xmax;
	//shape->Bounds.top		= flash_shape.ShapeBounds.Ymin;
	//shape->Bounds.bottom	= flash_shape.ShapeBounds.Ymax;

    // now tesselate the path
    return shape;
}

}

} 