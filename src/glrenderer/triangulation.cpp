/// @file   triangulation.cpp
///

#include "stdafx.h"
#include "triangulation.h"
#include "ogl_shape.h"
#include "swf_shape.h"
#include <iterator>

extern "C" 
{
#include <tesselator.h>
}

using namespace std;

namespace ogl
{

typedef std::vector<swf_redux::gfx::Edge> EdgeList_t;

static bool triangulate(const EdgeList_t & outerEdges,
    const std::vector<EdgeList_t> & innerEdges,
	std::vector<Triangle> & indicies,
	std::vector<Point > & verticies,
	float scaleFactor);

static bool triangulate(const EdgeList_t & outerEdges,
    std::vector<Triangle> & indicies,
	std::vector<Point > & verticies,
	float scaleFactor);

static void generate_texture_coords(const std::vector<Point > & a_Verticies, 
	std::vector<Point > & a_TexCoords);

static void set_mesh_material(const swf_redux::gfx::Path a_Path, CPUMesh & a_Mesh);
static void set_line_material(const swf_redux::gfx::Path a_Path, CPUMesh & a_Mesh);

static void merge_meshes(list<pair<Material, CPUMesh> > & a_Meshes,const CPUMesh & a_Mesh);

static void generate_polyline(const std::list<swf_redux::gfx::Edge> & a_Edges, 
					  std::vector<swf_redux::gfx::Edge> & a_StraightEdges,
					  float a_MaxError);

static void GetStrokeMesh(const std::vector<swf_redux::gfx::Edge> & a_Edges, 
						  const swf_redux::gfx::LineStyle & a_LineStyle, 
						  bool a_IsPathClosed,
						  CPUMesh & a_Mesh);

static void addPoints(const EdgeList_t & a_Edges, const swf_redux::gfx::Point<int32_t> & a_StartPoint,
		std::vector<swf_redux::gfx::Point<int32_t> > & a_Points);

bool triangulate_shape(const swf_redux::gfx::Shape & shape,
    CPUMeshes & meshes, float ErrorTolerance)
{
    for(size_t outIt = 0, numOuter = shape.outer_paths.size();
		outIt < numOuter;
		++outIt)
	{
		const swf_redux::gfx::Path & outerPath = shape.paths[shape.outer_paths[outIt]];
		if (outerPath.is_closed) {
			if (outerPath.edges.size() <= 2) {
				continue;
			}

			list<pair<Material, ogl::CPUMesh> > innerMeshes;

			// generate straight edges that approximate the curved edges.
			vector<swf_redux::gfx::Edge> outerStraights;
			generate_polyline(outerPath.edges, outerStraights, ErrorTolerance);

			ogl::CPUMesh mesh;
			list<size_t> holes, interiorShapes;
			// iterate over the inner shapes and determine if they define holes in the shape,
			// or just inner shapes.
			for(size_t inIt = 0, numInner = outerPath.inner_paths.size();
				inIt < numInner;
				++inIt)	
			{
				const swf_redux::gfx::Path & innerPath = shape.paths[outerPath.inner_paths[inIt]];
				if ((shape.is_glyph == false) && ((innerPath.is_closed == false) || (innerPath.is_ccw == 0))) {
					// defines a interior shape
					interiorShapes.push_back(inIt);
				} else if (innerPath.edges.size() > 2) {
					// defines a hole
					holes.push_back(inIt);
				}
			}
			if (holes.empty()) { // there is no holes in this shape
				if (outerStraights.size() > 2) {
					if (!triangulate(outerStraights, mesh.Indicies, mesh.VertexData, (float) shape.scale_factor)) {
						return false;
					}
				} else {
					// Todo, generate stroke.
				}
			} else { // they are holes in the shape.
				vector<EdgeList_t> innerPaths(holes.size());
				for(size_t hIt = 0, numHoles = holes.size();
					hIt < numHoles;
					++hIt)
				{
					const swf_redux::gfx::Path & hole = shape.paths[outerPath.inner_paths[hIt]];
					generate_polyline(hole.edges, innerPaths[hIt], ErrorTolerance);
				}
				// now triangulate the shape with the holes.
				if (!triangulate(outerStraights, innerPaths, mesh.Indicies, mesh.VertexData, (float) shape.scale_factor)) {
					return false;
				}
			}
			// Now generate texture coordinates.
			generate_texture_coords(mesh.VertexData, mesh.TextureCoords);
			set_mesh_material(outerPath, mesh);
			meshes.Meshes.push_back(mesh);

			// Now handle interior shapes that aren't holes
			for(list<size_t>::iterator it = interiorShapes.begin();
				it != interiorShapes.end();
				it++)
			{
				const swf_redux::gfx::Path & path = shape.paths[outerPath.inner_paths[*it]];
				if (path.is_closed) {
					CPUMesh innerMesh;
					vector<swf_redux::gfx::Edge> edges;
					generate_polyline(path.edges, edges, ErrorTolerance);
					if (edges.size() <= 2) {
						continue;
					}
					if (!triangulate(edges, innerMesh.Indicies, innerMesh.VertexData, (float) shape.scale_factor)) {
						return false;
					}
					if (!innerMesh.Indicies.empty() && !innerMesh.VertexData.empty()) {
						// TODO: determine why this is required, Why does it produce no vertex data?
						generate_texture_coords(innerMesh.VertexData, innerMesh.TextureCoords);
						set_mesh_material(path, innerMesh);
						merge_meshes(innerMeshes, innerMesh);
					}
				} else {
					CPUMesh innerMesh;
					vector<swf_redux::gfx::Edge> edges;
					generate_polyline(path.edges, edges, ErrorTolerance);
					GetStrokeMesh(edges, path.line_style, true, innerMesh);
					set_mesh_material(path, innerMesh);
					merge_meshes(innerMeshes, innerMesh);
				}
			}
			for(list<pair<Material, CPUMesh> >::iterator it = innerMeshes.begin();
				it != innerMeshes.end();
				it++)
			{
				meshes.Meshes.push_back(it->second);
			}
		} else {
			CPUMesh mesh;
			vector<swf_redux::gfx::Edge> edges;
			generate_polyline(outerPath.edges, edges, ErrorTolerance);
			GetStrokeMesh(edges, outerPath.line_style, true, mesh);
			set_line_material(outerPath, mesh);
			meshes.Meshes.push_back(mesh);
		}
	}

	for(size_t i = 0; i < meshes.Meshes.size(); ++i) {
		//calculateBounds(meshes[i]);	
	}
	return true;
}

static void translateColor(const swf_redux::gfx::Color & a_Color, float a_fColor[4])
{
	a_fColor[0] = float(a_Color.r) / 255;
	a_fColor[1] = float(a_Color.g) / 255;
	a_fColor[2] = float(a_Color.b) / 255;
	a_fColor[3] = float(a_Color.a) / 255;
}

static void set_line_material(const swf_redux::gfx::Path a_Path, CPUMesh & a_Mesh)
{
	a_Mesh.Material.MaterialType = Material::MATERIAL_SOLID_COLOR;
	translateColor(a_Path.line_style.color, a_Mesh.Material.Color);
}

template<class T>
inline Point normalize(const swf_redux::gfx::Point<T> & a_Vec)
{
	Point res;
	float length = sqrtf(float(a_Vec.x * a_Vec.x + a_Vec.y * a_Vec.y));
	res.x = float(a_Vec.x) / length;
	res.y = float(a_Vec.y) / length;
	return res;
}

static void get_normal(const swf_redux::gfx::Point<int32_t> & a_Start, 
					  const swf_redux::gfx::Point<int32_t> & a_End,
					  Point & a_Normal)
{
	int32_t dx = a_End.x - a_Start.x;
	int32_t dy = a_End.y - a_Start.y;
	a_Normal = normalize(swf_redux::gfx::Point<int32_t>(-dy, dx));
}

static void get_edge_corners(const swf_redux::gfx::Point<int32_t> & a_P1, 
						   const swf_redux::gfx::Point<int32_t> & a_P2,
						   Point a_Corners[4],
						   float a_HalfWidth)
{
	Point normal;
	get_normal(a_P1, a_P2, normal);
	normal = normal * a_HalfWidth;
	a_Corners[0] = Point(a_P1.x - normal.x, a_P1.y - normal.y) * (1.0f/20.0f);
	a_Corners[1] = Point(a_P1.x + normal.x, a_P1.y + normal.y) * (1.0f/20.0f);
	a_Corners[2] = Point(a_P2.x + normal.x, a_P2.y + normal.y) * (1.0f/20.0f);
	a_Corners[3] = Point(a_P2.x - normal.x, a_P2.y - normal.y) * (1.0f/20.0f);
}


static void GetStrokeMesh(const std::vector<swf_redux::gfx::Edge> & a_Edges, 
						  const swf_redux::gfx::LineStyle & a_LineStyle, 
						  bool a_IsPathClosed,
						  CPUMesh & a_Mesh)
{
	uint32_t numEdges = static_cast<uint32_t>(a_Edges.size());
	a_Mesh.VertexData.resize(4 * numEdges); /**< make room for the verticies */
	size_t indiciesIndex = 0;
	for(uint32_t i = 0, num = numEdges; i < num; ++i)
	{
		Point corners[4];
		const swf_redux::gfx::Edge & edge = a_Edges[i];
		/** calculate the corner positions of the rectangle defined by the edge */
		get_edge_corners(edge.start_pos, edge.stop_pos, &a_Mesh.VertexData[i*4], float(a_LineStyle.line_width)/ 2.0f);
		uint32_t base = i * 4;
		a_Mesh.Indicies.push_back(Triangle(base, base + 3, base + 1));
		a_Mesh.Indicies.push_back(Triangle(base + 1, base + 3, base + 2));

		if (i != (numEdges - 1)) {
			/** this is not the last one, handle the join between this edge and the next one */
			a_Mesh.Indicies.push_back(Triangle(base + 3, base + 4, base + 2));
			a_Mesh.Indicies.push_back(Triangle(base + 2, base + 3, base + 5));
		}
		if ((i == (numEdges - 1)) && (a_IsPathClosed)) {
			/** this is a last one in a closed path */
			a_Mesh.Indicies.push_back(Triangle(base + 3, 0, base + 2));
			a_Mesh.Indicies.push_back(Triangle(base + 2, base + 3, 1));
		}
	}
}

static void merge_meshes(list<pair<Material, CPUMesh> > & a_Meshes,const CPUMesh & a_Mesh)
{
	for(std::list<std::pair<Material, CPUMesh> >::iterator it = a_Meshes.begin();
		it != a_Meshes.end();
		it++)
	{
		if (it->first == a_Mesh.Material) {
			size_t initialCount = it->second.VertexData.size();
			std::copy(a_Mesh.VertexData.begin(), a_Mesh.VertexData.end(), std::back_inserter(it->second.VertexData));
			std::copy(a_Mesh.TextureCoords.begin(), a_Mesh.TextureCoords.end(), std::back_inserter(it->second.TextureCoords));

			for(size_t i = 0; i < a_Mesh.Indicies.size(); ++i)
			{
				Triangle tri;
				tri.p1 = a_Mesh.Indicies[i].p1 + static_cast<uint32_t>(initialCount);
				tri.p2 = a_Mesh.Indicies[i].p2 + static_cast<uint32_t>(initialCount);
				tri.p3 = a_Mesh.Indicies[i].p3 + static_cast<uint32_t>(initialCount);
				it->second.Indicies.push_back(tri);
			}
			return;
		}
	}
	a_Meshes.push_back(std::pair<Material, CPUMesh>(a_Mesh.Material, a_Mesh));
}

static void set_mesh_material(const swf_redux::gfx::Path a_Path, CPUMesh & a_Mesh)
{
	switch(a_Path.fill_stype.FillType) 
	{
	case swf_redux::gfx::FillStyle::SolidFill:
		a_Mesh.Material.MaterialType = Material::MATERIAL_SOLID_COLOR;
		translateColor(a_Path.fill_stype.u.color, a_Mesh.Material.Color);
		break;
	case swf_redux::gfx::FillStyle::LinearGradient:
	case swf_redux::gfx::FillStyle::RadialGradient:
		a_Mesh.Material.Gradient.Count	        = a_Path.fill_stype.u.gradient.num_control_points;
		if (a_Path.fill_stype.FillType == swf_redux::gfx::FillStyle::LinearGradient) {
			a_Mesh.Material.MaterialType		= Material::MATERIAL_LINEAR_GRADIENT;
		} else {
			a_Mesh.Material.MaterialType		= Material::MATERIAL_RADIAL_GRADIENT;
		}
		for(uint8_t i = 0; i < a_Path.fill_stype.u.gradient.num_control_points; ++i) {
			a_Mesh.Material.Gradient.Points[i].Ratio = a_Path.fill_stype.u.gradient.points[i].ratio;
			translateColor(a_Path.fill_stype.u.gradient.points[i].color, a_Mesh.Material.Gradient.Points[i].Color);
		}
		break;
	}
}

void generate_texture_coords(const std::vector<Point > & a_Verticies, 
	std::vector<Point > & a_TexCoords)
{
	Point minValue(FLT_MAX, FLT_MAX);
	Point maxValue(FLT_MIN, FLT_MIN);

	/** get the extreme values for the verticies */
	a_TexCoords.resize(a_Verticies.size());
	for(size_t i = 0, count = a_Verticies.size(); i < count; ++i) {
		minValue.x = std::min(minValue.x, a_Verticies[i].x);
		minValue.y = std::min(minValue.y, a_Verticies[i].y);
		maxValue.x = std::max(maxValue.x, a_Verticies[i].x);
		maxValue.y = std::max(maxValue.y, a_Verticies[i].y);
	}

	float w = maxValue.x - minValue.x;
	float h = maxValue.y - minValue.y;

	/** calculate the texture coordinate of each vertex */
	for(size_t i = 0, count = a_Verticies.size(); i < count; ++i) {
		a_TexCoords[i].x = (a_Verticies[i].x - minValue.x) / w;
		a_TexCoords[i].y = (a_Verticies[i].y - minValue.y) / h;
	}
}

static bool triangulate(const EdgeList_t & outerEdges,
					   const std::vector<EdgeList_t> & innerEdges,
					   std::vector<Triangle> & indicies,
					   std::vector<Point > & verticies,
					   float scaleFactor)
{
	if (outerEdges.empty()) {
		return false;
	}
	vector<swf_redux::gfx::Point<int32_t> > points;
	addPoints(outerEdges, outerEdges[0].start_pos, points);
	vector<swf_redux::gfx::Point<float> > fPoints(points.size());

	// scale the points correctly.
	float scaleInv = 1.0f / scaleFactor;
	for(size_t i = 0; i < fPoints.size(); ++i) {
		fPoints[i].x = float(points[i].x) * scaleInv;
		fPoints[i].y = float(points[i].y) * scaleInv;
	}

	// scale the interior points
	vector<vector<swf_redux::gfx::Point<float> > > fInnerPoints(innerEdges.size());
	for(size_t i = 0; i < innerEdges.size(); ++i) {
		vector<swf_redux::gfx::Point<int32_t> > InnerPoints;
		addPoints(innerEdges[i], innerEdges[i][0].start_pos, InnerPoints);
		vector<swf_redux::gfx::Point<float> > & ref = fInnerPoints[i];
		ref.resize(InnerPoints.size());
		for(size_t i = 0; i < InnerPoints.size(); ++i) {
			ref[i].x = float(InnerPoints[i].x) * scaleInv;
			ref[i].y = float(InnerPoints[i].y) * scaleInv;
		}
	}
	
	// create a tesselator instance.
	TESStesselator * tess = tessNewTess(0);
	if (!tess) {
		return false;
	}
	// add the outer points
	tessAddContour(tess, 2, &fPoints[0], 8, (int) fPoints.size());
	// add the interior points
	for(size_t i = 0; i < fInnerPoints.size(); ++i) {
		tessAddContour(tess, 2, &fInnerPoints[i][0], 8, (int)fInnerPoints[i].size()); 
	}
	int res = tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, NULL);
	if (res == 0) {
		tessDeleteTess(tess);
		return false;
	}

	const float* verts  = tessGetVertices(tess);
	const int* elems    = tessGetElements(tess);
	const int nverts    = tessGetVertexCount(tess);
	const int nelems    = tessGetElementCount(tess);

	// copy indicies data
	indicies.resize(nelems);
	if (nelems) {
		memcpy(&indicies[0], elems, nelems * sizeof(int) * 3);
	}
	// copy vertex data
	verticies.resize(nverts);
	if (nverts) {
		memcpy(&verticies[0], verts, nverts * sizeof(float) * 2);
	}
	tessDeleteTess(tess);
	return true;
}

static bool triangulate(const EdgeList_t & outerEdges,
					   std::vector<Triangle> & indicies,
					   std::vector<Point> & verticies,
					   float scaleFactor)
{
	vector<EdgeList_t> inner;
	return triangulate(outerEdges, inner, indicies, verticies, scaleFactor);
}

static void addPoints(const EdgeList_t & a_Edges, const swf_redux::gfx::Point<int32_t> & a_StartPoint,
		std::vector<swf_redux::gfx::Point<int32_t> > & a_Points)
{
	size_t startIndex = 0, numEdges = a_Edges.size();
	for(size_t i = 0; i < numEdges; ++i) {
		if (a_StartPoint == a_Edges[i].start_pos) {
			startIndex = i;
		}
	}
	/** add points [startIndex, end] */
	for(size_t i = startIndex; i < numEdges; ++i) {
		a_Points.push_back(a_Edges[i].start_pos);
	}
	/** add points [0, startIndex] */
	for(size_t i = 0; i < startIndex; ++i) {
		a_Points.push_back(a_Edges[i].start_pos);
	}
}

#define MAX_SUBDIVISION_DEPTH (256)

void subdivide(float t0,
    float t1, 
    Bezier<float> & bezier, 
	float a_MaxError,
    std::vector<swf_redux::gfx::Edge> & a_StraightEdges, 
    size_t a_RecursiveCount)
{
	float mid = (t0 + t1) / 2.0f;
	swf_redux::gfx::Point<float> midBezier = bezier.Get(mid); /**< actual midpoint on Bezier */
	swf_redux::gfx::Point<float> p0 = bezier.Get(t0);
	swf_redux::gfx::Point<float> p1 = bezier.Get(t1);

	swf_redux::gfx::Point<float> diff = p1 - p0;
	swf_redux::gfx::Point<float> midLine = p0 + (diff * 0.5f); /**< midpoint on the current line segment */

	/** Calculate the error of this line segment using the Manhattan distance between the
		midpoint on the Bezier curve and the midpoint on the current line segment */
	float distance = fabs(midBezier.x - midLine.x) + fabs(midBezier.y - midLine.y);
		
	if ((distance <= a_MaxError) || /**< The error is below the limit */
		(a_RecursiveCount > MAX_SUBDIVISION_DEPTH)) { /**< Or we have reach the recursion depth limit */
		swf_redux::gfx::Edge edge;
		edge.is_straight = 1;
		/** round to integer twips */ 
		edge.start_pos	= swf_redux::gfx::Point<int32_t>(int32_t(p0.x), int32_t(p0.y));
		edge.stop_pos	= swf_redux::gfx::Point<int32_t>(int32_t(p1.x), int32_t(p1.y));
		a_StraightEdges.push_back(edge);
	} else { /**< Need to subdivide */
		subdivide(t0, mid, bezier, a_MaxError, a_StraightEdges, a_RecursiveCount + 1);
		subdivide(mid, t1, bezier, a_MaxError, a_StraightEdges, a_RecursiveCount + 1);
	}
}
	
void subdivideCurvedEdge(const swf_redux::gfx::Edge & a_Edge, 
    std::vector<swf_redux::gfx::Edge> & a_StraightEdges, float a_MaxError)
{
	Bezier<float> bezier(swf_redux::gfx::Point<float>(float(a_Edge.start_pos.x), float(a_Edge.start_pos.y)),
		swf_redux::gfx::Point<float>(float(a_Edge.control_pos.x), float(a_Edge.control_pos.y)),
		swf_redux::gfx::Point<float>(float(a_Edge.stop_pos.x), float(a_Edge.stop_pos.y)));

	subdivide(0.0f, 1.0f, bezier, a_MaxError, a_StraightEdges, 0);
}

static void generate_polyline(const std::list<swf_redux::gfx::Edge> & a_Edges, 
					  std::vector<swf_redux::gfx::Edge> & a_StraightEdges,
					  float a_MaxError)
{
    for(std::list<swf_redux::gfx::Edge>::const_iterator it = a_Edges.begin();
        it != a_Edges.end();
        it++)
    {
		if (it->is_straight) {
			a_StraightEdges.push_back(*it);
		} else {
			subdivideCurvedEdge(*it, a_StraightEdges, a_MaxError);
		}
	}
}


} // namespace ogl
