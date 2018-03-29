#ifndef _SWF_ALLOCATOR_H_
#define _SWF_ALLOCATOR_H_

#include <stdint.h>
#include <cstdlib>
#include <assert.h>

#pragma warning( disable : 4200 )

namespace swf_redux
{

// represents a chunk of allocated memory.
struct Page
{
    static const size_t _page_size = 8192;

    uint8_t *           _data;
    Page *              _next_page;
};

// Used by allocators to allocate a 8192 chunk of memory
// called a page.
class PageAllocator
{
public:
    // allocates a single page.
    Page * allocate() { return 0; }
    // returns a allocated page to the allocator.
    void deallocate(Page *) {}
};

/**
 * Interface for a generic allocator
 */
class IAllocator
{
public:
    virtual void * allocate(size_t count) = 0;
    virtual void deallocate(void * memory) = 0;
};

class MallocAllocator : public IAllocator
{
public:
    virtual void * allocate(size_t count)
    {
        return malloc(count);
    }

    virtual void deallocate(void * memory)
    {
        free(memory);
    }
};

/// Fixed size allocator, allocates objects of a specific size.
/// Alignment must be a power of two.
template<size_t element_size>
class FixedSizeAllocator
{
    struct PageSuffix {
        uint32_t    _num_allocated;
        uint32_t    _bitmap[0];

    protected:
        PageSuffix(const PageSuffix &);
        PageSuffix & operator=(const PageSuffix &);
    };

public:
    FixedSizeAllocator(PageAllocator & allocator) : 
        _page_allocator(allocator),
        _page_list(0)
    {
        // the maximum number of elements that could be stored in a page.
        size_t max_elements = Page::_page_size / element_size;
        size_t suffix_size  = ((max_elements % 32) ? ((max_elements / 32) + 1) : (max_elements / 32)) + 1;
        // now calculate how many objects that can be stored if we take the suffix into account.
        _elements_per_page  = (Page::_page_size - suffix_size) / element_size;
        // now save the offset to the suffix.
        _suffix_offset      = Page::_page_size - (_elements_per_page * element_size);
    }

    virtual ~FixedSizeAllocator()
    {
        Page * current_page = _page_list;
        while(current_page) {
            // save a pointer to the next page before we return it.
            Page * next_page = current_page->_next_page;
            // return the page to the page allocator
            _page_allocator.deallocate(current_page);
            // continue with the next page.
            current_page = next_page;
        }
    }

    // allocates a element.
    void * allocate()
    {
        Page * current_page = _page_list;
        while(current_page)
        {
            // the suffix is located at the end of the page.
            PageSuffix * suffix = (PageSuffix *) (current_page->_data + Page::_page_size) - _suffix_offset;
            if (suffix->_num_allocated < _elements_per_page) {
                // there is one or more empty slots in this page.
                return (void *)find_and_mark(current_page, suffix);
            }
        }
        return 0;
    }

    // deallocates a element.
    void deallocate(void * element)
    {
        Page * current_page = _page_list;
        while(current_page) {
            // check if the element is inside this page
            if ((element >= current_page->_data) ||
                (element < (current_page->_data + Page::_page_size)))
            {
                PageSuffix * suffix = free_element(element, current_page);
                return;
            }
            current_page = current_page->_next_page;
        }
    }

protected:
    /// prevent a allocator to be copied.
    FixedSizeAllocator(const FixedSizeAllocator &);
    FixedSizeAllocator & operator=(const FixedSizeAllocator &);

    PageSuffix * free_element(void * element, Page * page)
    {
        PageSuffix * suffix = (PageSuffix *) (page->_data + Page::_page_size) - _suffix_offset;
        assert(suffix->_num_allocated > 0);
        // clear the position in the bitmap.
        size_t index = ((uint8_t *)element - page->_data) / element_size;
        suffix->_bitmap[index / 32] &= (1 << (index % 32));
        // decrement the number of elements.
        --suffix->_num_allocated;
        return suffix;
    }

    void * find_and_mark(Page * page, PageSuffix * suffix)
    {
        assert(suffix->_num_allocated < _elements_per_page);
        for(size_t bitmap_index = 0; 
            bitmap_index < _elements_per_page;
            bitmap_index += 32)
        {
            uint32_t & dw = suffix->_bitmap[bitmap_index >> 5];
            if (dw != UINT32_MAX)
            {
                for(uint32_t mask = (1 << 31), index = 32; 
                    mask; 
                    mask >>= 1, --index)
                {
                    if (!(dw & mask)) {
                        // this position is cleared, mark it as allocated
                        dw |= mask;
                        // increment the number of allocate objects.
                        ++suffix->_num_allocated;
                        // return the correct pointer.
                        return page->_data + (bitmap_index + index) * element_size;
                    }
                }
            }
        }
        // this should never happen.
        assert(false);
        return 0;
    }

private:
    /// the page allocator used to request new pages from.
    PageAllocator & _page_allocator;
    /// linked list of all the pages managed by this allocator.
    Page *          _page_list;
    uint32_t        _suffix_offset      : 10;     /// offset to the suffix, from the page end!
    uint32_t        _elements_per_page  : 10;     /// the number of elements per page.
};

}

#endif