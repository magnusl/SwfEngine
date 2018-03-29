#ifndef MMAPSOURCE_H
#define MMAPSOURCE_H

#include "swf_source.h"

namespace swf_redux
{

namespace io
{

class MMapSource_impl;
/// Source which uses a memory mapped file
///
class MMapSource : public Source
{
public:
	MMapSource(const wchar_t * a_FilePath);
	MMapSource(const std::wstring & a_FilePath);
	virtual ~MMapSource();

    /// source interface
    virtual bool end_of_file() const;
    virtual bool read(void * dst, size_t count);
    virtual bool skip(size_t count);

protected:
    MMapSource_impl * impl;
};

} // namespace io

} // namespace swf_redux

#endif