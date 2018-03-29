#include "swf_types.h"
#include "tag_decoder.h"
#include "swf_decode.h"
#include "swf_tags.h"
#include "swf_exception.h"

namespace swf_redux
{

namespace swf
{

/// Parse a SWF shape definition
///
bool parse_defineshape4(TagDecoder & io, DefinedShape4 & shape)
{
    try {
	    shape.ShapeId = io.get_u16();
	    Decode<RECT>((io::Source &)io, shape.ShapeBounds);
	    if (io.tag_code() == SwfTag_DefineShape4) {
		    Decode<RECT>((io::Source &)io, shape.EdgesBounds);
		    io.get_bits(5);
		    shape.UsesFillWindingRule	= io.get_bits(1);
		    shape.UsesNonScalingStrokes = io.get_bits(1);
		    shape.UsesScalingStrokes	= io.get_bits(1);
	    }
	    io.assure_alignment();
	    Decode<ShapeWithStyle>(io, shape.Shapes);
    } catch(...) {
        return false;
    }
    return true;
}

}

}