#ifndef TAG_DECODER_H
#define TAG_DECODER_H

#include "swf_source.h"
#include <stdint.h>
#include <vector>

namespace swf_redux
{

class TagDecoder : public io::Source
{
public:
    TagDecoder(io::Source & source);

    uint32_t		tag_length() const;
	uint16_t		tag_code() const;
	uint32_t		tag_offset() const;
    
    // Source interface
    virtual bool    end_of_file() const;
    virtual bool    read(void * dst, size_t count);
    virtual bool    skip(size_t count);

protected:
    TagDecoder(const TagDecoder &);
    const TagDecoder & operator=(const TagDecoder &);

    std::vector<uint8_t>    _data;
	uint32_t	            _tag_length;
	uint16_t	            _tag_code;
	uint32_t	            _offset;
};

}

#endif
