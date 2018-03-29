#ifndef MMAP_POSIX_H
#define MMAP_POSIX_H

#include "swf_source.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "unistd.h"

namespace swf_redux
{

namespace io
{

/// Source which uses a memory mapped file
///
class MMapPosix : public Source
{
public:
	MMapPosix(const char * path) : _offset(0)
	{
		_fd = open(path, O_RDONLY);
		if (_fd < 0) {
			throw std::runtime_error("Failed to open file.");
		}

		struct stat st;
		if (fstat(_fd, &st) < 0) {
			close(_fd);
			throw std::runtime_error("Failed to fstat file.");
		}

		_size 	= st.st_size;
		_mem 	= mmap(NULL, _size, PROT_READ, MAP_ANON, _fd, 0);
		if (!_mem) {
			close(_fd);
			throw std::runtime_error("Failed to memory map the file.");
		}
	}

	virtual ~MMapPosix()
	{
		munmap(_mem, _size);
		close(_fd);
	}
    
    /// source interface
    virtual bool end_of_file() const 
    {
    	return _offset >= _size;
    }

    virtual bool read(void * dst, size_t count)
    {
    	if ((_offset + count) > _size) {
    		return false;
    	}
    	memcpy(dst, ((char *) _mem) + _offset, count);
    	_offset += count;
    	return true;
    }

    virtual bool skip(size_t count)
    {
    	if ((_offset + count) > _size) {
    		return false;
    	}
    	_offset += count;
    	return true;
    }

protected:
    size_t 	_size, _offset;
    int _fd;
    void * _mem;
};

} // namespace io

} // namespace swf_redux

#endif