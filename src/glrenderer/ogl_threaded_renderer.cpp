#include "stdafx.h"
#include "ogl_threaded_renderer.h"
#include "triangulation_job.h"
#include "ogl_quadtree.h"
#include "ogl_drawable.h"
#include "ogl_depgraph.h"

using namespace std;
using namespace Eigen;

namespace ogl
{

struct DrawItem {
    std::shared_ptr<Shape>  shape;
    Eigen::Matrix3f         transformation;
    float                   alpha;
};

struct CacheItem {
    std::list<std::shared_ptr<Shape> > cached;       
};


class ThreadedRenderer::Impl
{
public:
    Impl(size_t num_threads) : _pool(num_threads)
    {
    }

    ~Impl();

    std::shared_ptr<Shape> get_cached(std::shared_ptr<swf_redux::gfx::Shape>,
        const Eigen::Matrix3f & transform);
    void draw(std::shared_ptr<swf_redux::gfx::Shape> shape, 
        const Eigen::Matrix3f & transform, 
        float alpha);
    void flush();

protected:
    void draw(DrawItem & item);
    
    void draw(std::shared_ptr<Shape> shape, const Eigen::Matrix3f & transform, float alpha);
    void draw(std::shared_ptr<Mesh> mesh, const Eigen::Matrix3f & transform, float alpha);
    void draw(const ogl::Drawable & drawable, const Eigen::Matrix3f & transformation);

private:
    swf_redux::os::ThreadPool           _pool;
    list<DrawItem>                      _draw_queue;
    list<CacheItem *>                   _caches;
    shared_ptr<QuadTree<uint32_t, 8> >  _qtree;

    vector<shared_ptr<DependencyNode<Drawable> > > _drawn_objects;
};

ThreadedRenderer::ThreadedRenderer(size_t num_threads)
{
    _impl = new (std::nothrow) ThreadedRenderer::Impl(num_threads);
}

ThreadedRenderer::~ThreadedRenderer()
{
    delete _impl;
}

std::shared_ptr<Shape> ThreadedRenderer::Impl::get_cached(
    std::shared_ptr<swf_redux::gfx::Shape> shape,
    const Eigen::Matrix3f & transform)
{
    float error             = 0.0f;    
    CacheItem * shape_cache = (CacheItem *) shape->user_data;

    if (NULL == shape->user_data) {
        shape_cache = new (std::nothrow) CacheItem();
        _caches.push_back(shape_cache);
    }

    for(list<shared_ptr<Shape> >::iterator it = shape_cache->cached.begin();
        it != shape_cache->cached.end();
        it++)
    {
        if ((*it)->error_tolerance <= error) {
            return *it;
        }
    }
    shared_ptr<Shape> new_shape = make_shared<Shape>();
    new_shape->error_tolerance  = error;
    shape_cache->cached.push_back(new_shape);
    _pool.submit_job(make_shared<TriangulationJob>(new_shape, shape, transform));

    return new_shape;
}

void ThreadedRenderer::draw(std::shared_ptr<swf_redux::gfx::Shape> shape, 
        const Eigen::Matrix3f & transform, 
        float alpha)
{
    _impl->draw(shape, transform, alpha);
}

void ThreadedRenderer::Impl::draw(std::shared_ptr<swf_redux::gfx::Shape> shape, 
        const Eigen::Matrix3f & transform, 
        float alpha)
{
    /// get the cached shape. This will trigger a triangulation
    /// job if we didn't find a suitable cached version.
    std::shared_ptr<Shape> ogl_shape = get_cached(shape, transform);
    if (ogl_shape)
    {
        /// add it to the draw queue
        DrawItem item;
        item.shape          = ogl_shape;
        item.alpha          = alpha;
        item.transformation = transform;
        _draw_queue.push_back(item);
    }
}

void ThreadedRenderer::flush(void)
{
    _impl->flush();
}

void ThreadedRenderer::Impl::flush(void)
{
    _pool.wait();
    for(list<DrawItem>::iterator it = _draw_queue.begin(); it != _draw_queue.end(); it++) {
        draw( *it );
    }
    _draw_queue.clear();
}

void ThreadedRenderer::Impl::draw(DrawItem & item)
{
    if (item.shape->cpu_meshes)
    {
        create_ogl_shape(item.shape->cpu_meshes, item.shape);
        item.shape->cpu_meshes.reset();
    }
    draw(item.shape, item.transformation, item.alpha);
}

void ThreadedRenderer::Impl::draw(std::shared_ptr<Shape> shape,
    const Eigen::Matrix3f & transform,
    float alpha)
{
    for(list<shared_ptr<Mesh> >::const_iterator it = shape->ogl_meshes.begin();
        it != shape->ogl_meshes.end();
        it++)
    {
        draw(*it, transform, alpha);
    }
}

static void get_transformed(const Rect<float> & a_Original,
		const Eigen::Matrix3f & a_Transformation,
		Rect<float> & a_TransformedBoundry)
{
    Vector3f topLeft        = a_Transformation * Vector3f(a_Original.left, a_Original.top, 1);
    Vector3f topRight       = a_Transformation * Vector3f(a_Original.right, a_Original.top, 1);
    Vector3f bottomLeft     = a_Transformation * Vector3f(a_Original.left, a_Original.bottom, 1);
    Vector3f bottomRight    = a_Transformation * Vector3f(a_Original.right, a_Original.bottom, 1);

	/// construct the boundry based on the new extreme values
	a_TransformedBoundry.left	= min(topLeft.x(), min(topRight.x(), min(bottomRight.x(), bottomLeft.x())));
	a_TransformedBoundry.right	= max(topLeft.x(), max(topRight.x(), max(bottomRight.x(), bottomLeft.x())));
	a_TransformedBoundry.top	= min(topLeft.y(), min(topRight.y(), min(bottomRight.y(), bottomLeft.y())));
	a_TransformedBoundry.bottom = max(topLeft.y(), max(topRight.y(), max(bottomRight.y(), bottomLeft.y())));
}

void ThreadedRenderer::Impl::draw(std::shared_ptr<Mesh> mesh, 
    const Eigen::Matrix3f & transform,
    float alpha)
{
    
} 

void ThreadedRenderer::Impl::draw(const ogl::Drawable & drawable,
    const Eigen::Matrix3f & transformation)
{
    /// transform the bounds
    Rect<float> transformed;
    get_transformed(drawable.Bounds, transformation, transformed);

    /// insert it into the quadtree and get overlapping objects
    list<uint32_t> overlapping;
    _qtree->insert(transformed, _drawn_objects.size(), overlapping);

    // add dependencies to overlapping objects
    shared_ptr<DependencyNode<Drawable> > node = std::make_shared<DependencyNode<Drawable> >(drawable);
    for(list<uint32_t>::iterator it = overlapping.begin(); it != overlapping.end(); it++) {
        node->AddDependency(_drawn_objects[*it].get());
    }
    // sync(node)
    _drawn_objects.push_back(node);
}

}