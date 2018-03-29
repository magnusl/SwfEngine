#ifndef SWF_DECODE_H
#define SWF_DECODE_H

#include "tag_decoder.h"
#include "swf_types.h"

namespace swf_redux
{
namespace swf
{
    
template<class T>
void Decode(TagDecoder &, T &)
{
	//static_assert(false, "Missing type specific implementation of Decode().");
}
	
template<class T>
void Decode(io::Source &, T & t)
{
	//static_assert(false, "");
}

template<>
void Decode<RECT>(io::Source &, RECT &);

template<>
void Decode<ShapeWithStyle>(TagDecoder &, ShapeWithStyle &);

template<>
void Decode<Shape>(TagDecoder & io, Shape & shape);

template<>
void Decode<Matrix_Fixed2x3>(io::Source &, Matrix_Fixed2x3 &);

template<>
void Decode<StyleChangedRecord>(TagDecoder &, StyleChangedRecord & stylechange);

template<>
void Decode(io::Source &, FileHeader & header);

template<>
void Decode(io::Source & io, CXFormWithAlpha & cxform);

template<>
void Decode<GRADRECORD>(TagDecoder & io, GRADRECORD & gr);

template<>
void Decode<DropShadowFilter>(io::Source & io, DropShadowFilter & drop);

template<>
void Decode<BlurFilter>(io::Source & io, BlurFilter & drop);

template<>
void Decode<GlowFilter>(io::Source & io, GlowFilter & drop);

template<>
void Decode<ColorMatrixFilter>(io::Source & io, ColorMatrixFilter & colorMatrix);

template<>
void Decode<Filter>(io::Source & io, Filter & a_Filter);
	
template<class T>
void DecodeArray(TagDecoder & io, std::vector<T> & arr)
{
	uint16_t count = io.get_u8();
	count = (count == 0xff) ? io.get_s16() : count;
	//arr.resize(count);
	for(uint16_t i = 0; i < count; i++) {
		T item;
		Decode<T>(io, item);
		arr.push_back(item);
	}
}

void SetBackgroundColor(TagDecoder & tag, RGB & color);

template<>
void Decode<RGBA>(io::Source & io, RGBA & rgba);
	
template<>
void Decode<RGB>(io::Source & io, RGB & rgba);

template<>
void Decode<MorphFillStyle>(TagDecoder & io, MorphFillStyle &);

template<>
void Decode<MorphLineStyle>(TagDecoder & io, MorphLineStyle &);

}

}

#endif