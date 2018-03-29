#ifndef _SWF_SOURCE_H_
#define _SWF_SOURCE_H_

#include <string>
#include <stdint.h>

namespace swf_redux
{

namespace io
{

/// A source is used to decode a resource.
///
class Source
{
public:

    Source();
    virtual ~Source();

    /// Interface that needs to be implemented by subclasses
    virtual bool end_of_file() const            = 0;
    virtual bool read(void * dst, size_t count) = 0;
    virtual bool skip(size_t count)             = 0;

    uint32_t        get_bits(size_t num_bits);
    int32_t         get_signed(size_t num_bits);
    uint8_t         get_u8();
    uint16_t        get_u16();
    uint32_t        get_u32();
    uint32_t        get_u30();
    int16_t         get_s16();
    int32_t         get_s24();
    int32_t         get_s32();
    int16_t         get_fixed8();
    int32_t         get_fixed16(size_t num_bits);
    float           get_float();
    double          get_double();
    std::string     get_string();
    void            align();
    void            assure_alignment();

protected:
    size_t			_offset; 
	uint8_t			_cached_value;
};

} // namespace io

} // namespace swf_redux

#endif