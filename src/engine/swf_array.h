#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "swf_allocator.h"

namespace swf_redux
{

template<class T>
struct Array
{
    Array(IAllocator & allocator) :
        _elements(0),
        _used(0),
        _reserved(0),
        _allocator(allocator)
    {
    }

    T & operator[](size_t index) { return _elements[index]; }

    T *             _elements;
    size_t          _used;
    size_t          _reserved;
    IAllocator &    _allocator;
};

namespace array
{
    template<class T>
    inline size_t size(const Array<T> & array)
    {
        return array._used;
    }

    template<class T>
    void push_back(Array<T> & array, const T & element)
    {
        if (array._used < array._reserved) {
            
        }
    }

    template<class T>
    void pop_back()
    {

    }
}

}

#endif