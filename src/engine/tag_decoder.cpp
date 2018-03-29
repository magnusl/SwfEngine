#include "tag_decoder.h"
#include "swf_exception.h"

namespace swf_redux
{

TagDecoder::TagDecoder(io::Source & source) : _offset(0)
{
    uint16_t TagCodeAndLength, length;

	TagCodeAndLength = source.get_u16();
	// The tag code is encoded in the upper 10 bits
	_tag_code = (TagCodeAndLength >> 6);
	length = TagCodeAndLength & 0x3f;
	if (length != 0x3f) {
		_tag_length = static_cast<int32_t>(length);
	} else {
		_tag_length = source.get_u32();
	}
    // read the data
    if (_tag_length) {
        _data.resize(_tag_length);
        if (!source.read(&_data[0], _tag_length)) {
            throw ParseError("Failed to read tag payload.");
        }
    }
}

uint32_t TagDecoder::tag_length() const
{
    return _tag_length;
}

uint16_t TagDecoder::tag_code() const
{
    return _tag_code;
}

uint32_t TagDecoder::tag_offset() const
{
    return _offset;
}

bool TagDecoder::end_of_file() const
{
    return _offset >= _tag_length;
}

bool TagDecoder::read(void * dst, size_t count)
{
    if ((_offset + count) > _tag_length) {
        return false;
    }
    memcpy(dst, &_data[_offset], count);
    _offset += count;
    return true;
}

bool TagDecoder::skip(size_t count)
{
    if ((_offset + count) > _tag_length) {
        return false;
    }
    _offset += count;
    return true;
}

} // namespace swf_redux