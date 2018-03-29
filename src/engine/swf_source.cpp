#include "swf_source.h"

namespace swf_redux
{

namespace io
{

Source::Source() : _offset(0), _cached_value(0)
{
    // empty
}

Source::~Source()
{
    // empty
}

uint32_t Source::get_u30()
{
	uint32_t dw = get_u8();
	if (!(dw & 0x80))			return dw;
	dw			= (dw & 0x7f) | (static_cast<uint32_t>(get_u8()) << 7);
	if (!(dw & 0x4000))			return dw;
	dw			= (dw & 0x3fff) | (static_cast<uint32_t>(get_u8()) << 14);
	if (!(dw & 0x200000))		return dw;
	dw			= (dw & 0x001FFFFF) | (static_cast<uint32_t>(get_u8()) << 21);
	if (!(dw & 0x10000000))		return dw;
	dw			= (dw & 0x0FFFFFFF) | (static_cast<uint32_t>(get_u8()) << 28);

	return dw;
}

uint8_t Source::get_u8()
{
	align();
	uint8_t b;
	if (!read(&b, sizeof(uint8_t))) {
		throw std::runtime_error("Failed to read byte from source.");
	}
	return b;
}

uint16_t Source::get_u16()
{
	align();
	uint16_t w;
	if (!read(&w, sizeof(uint16_t))) {
		throw std::runtime_error("Failed to read word from source.");
	}
	return w;
}

int16_t Source::get_s16()
{
	align();
	int16_t w;
	if (!read(&w, sizeof(int16_t))) {
		throw std::runtime_error("Failed to read word from source.");
	}
	return w;
}

uint32_t Source::get_u32()
{
	align();
	uint32_t dw;
	if (!read(&dw, sizeof(uint32_t))) {
		throw std::runtime_error("Failed to read dword from source.");
	}
	return dw;
}

int32_t Source::get_s24()
{
	align();
	uint8_t v[3];
	v[0] = get_u8();
	v[1] = get_u8();
	v[2] = get_u8();

	int32_t s24 = (((unsigned) v[2]) << 16) | (((unsigned) v[1]) << 8) | v[0];
	if (s24 & 0x800000) {
		return (s24 | 0xff000000);
	} else {
		return s24;
	}
}

int32_t Source::get_s32()
{
	align();
	size_t count = 0;
	uint32_t dw = 0;
	uint8_t b;
	do {
		b = get_u8();
		dw	= (((uint32_t) (b & 0x7f)) << count) | dw;
		count += 7;
	} while(b & 0x80);

	if (dw & 0x40) { // sign-extend.
		return (int32_t) (dw | (0xffffffff << (32 - count)));
	}
	return dw;
}

double Source::get_double()
{
	align();
	double d;
	if (!read(&d, sizeof(d))) {
		throw std::runtime_error("failed to read double from source.");
	}
	return d;
}

float Source::get_float()
{
	align();
	float f;
	if (!read(&f, sizeof(f))) {
		throw std::runtime_error("Failed to read float from source.");
	}
	return f;
}

std::string Source::get_string(void)
{
	std::string str;
	uint8_t c = get_u8();
	while(c != 0) {
		str += (char) c;
		c = get_u8();
	}
	return str;
}

/**
 * \brief Reads a variable length encoded signed 16.16 fixed point number.
 */
int32_t Source::get_fixed16(size_t num_bits)
{
	uint32_t w = get_bits(num_bits);
	if (w & (1 << (num_bits - 1))) {
		return (int32_t) (w | (0xffffffff << num_bits));	// sign extend.
	}
	return (int32_t) w;
}

int16_t Source::get_fixed8()
{
	int16_t w = get_signed(16);
	return w;
}

/**
 * Reads up to 32 bits from the bitstream and returns them as the lower bits of a uint32_t.
 */
uint32_t Source::get_bits(size_t count) 
{
	uint32_t dw = 0;
	while(count > 0) 
	{
		if (_offset) {
			/** the number of bits to read from the current cached value */
			size_t rc = count > _offset ? _offset : count;
			size_t sc = _offset - rc;
			dw = (dw << rc) | ((_cached_value >> sc) & (0xff >> (8 - rc)));
			count   -= rc;
			_offset -= rc;
		} else {
			if (!read(&_cached_value, sizeof(uint8_t))) {
				throw std::runtime_error("Failed to read data from source.");
			}
			_offset = 8;
		}
	}
	return dw;
}

int32_t Source::get_signed(size_t count)
{
	uint32_t dw = get_bits(count);
	if (count > 0) {
		if (dw & (1 << (count-1))) {
			uint32_t mask = (0xffffffff << count);
			return (int32_t) (dw | mask);
		}
	}
	return dw;
}

void Source::align()
{
	/** ignore any remaining bits in the cached value */
	_offset = 0;
}

void Source::assure_alignment()
{
	if (_offset) {
		throw std::runtime_error("Not byte aligned.");
	}
}

} // namespace io

} // namespace swf_redux
