#ifndef OGL_RECT_H
#define OGL_RECT_H

namespace ogl
{

template<class T>
struct Rect 
{
	Rect()
	{}

	Rect(T a_Left, T a_Right, T a_Top, T a_Bottom) :
		left(a_Left), 
        right(a_Right), 
        top(a_Top),
        bottom(a_Bottom)
	{}

    int inside(const Rect<T> & a_Rhs) const 
    {
        if (a_Rhs.right <= left) return -1;
		if (a_Rhs.left >= right) return -1;
		if (a_Rhs.bottom <= top) return -1;
		if (a_Rhs.top >= bottom) return -1;

		if ((a_Rhs.left >= left) && (a_Rhs.right <= right) &&
            (a_Rhs.top >= top) && (a_Rhs.bottom <= bottom)) 
		{
            return 1;
        }
        return 0;
    }

    T left, right, top, bottom;
};

} // namespace ogl

#endif