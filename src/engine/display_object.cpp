#include "display_object.h"

using namespace Eigen;

namespace swf_redux
{

namespace flash
{

namespace display
{

/// Get the display objects global transformation.
void get_global_transformation(DisplayObject * object,
    Eigen::Matrix3f & transformation)
{
    if (object->_parent) {
        transformation = get_local_transformation(object);
    } else {
        Matrix3f mat;
        get_global_transformation(object, mat);
        mat = mat * get_local_transformation(object);
    }
}

/// Get the display objects local transformation.
const Eigen::Matrix3f & get_local_transformation(DisplayObject * object)
{
    if (object->_dirty) {
        object->_local_transform = Eigen::Matrix3f::Identity();
        if (object->_rotation) {
			object->_local_transform *= AngleAxis<float>((object->_rotation * 0.0174532925f), 
            Vector3f(0, 0, -1)).toRotationMatrix();
        }
        object->_local_transform.col(2) = Vector3f(object->_position_x, object->_position_y, 1.0f);
        object->_local_transform = Scaling(object->_scale_x, object->_scale_y, 1.0f);
    }
    return object->_local_transform;
}

}

}

}