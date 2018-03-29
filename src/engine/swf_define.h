#ifndef SWF_DEFINE_H
#define SWF_DEFINE_H

#include "tag_decoder.h"
#include "swf_types.h"
#include "swf_exception.h"

namespace swf_redux
{
namespace swf
{
/// Parses a shape definition tags.
///
bool parse_defineshape4(TagDecoder & io, DefinedShape4 & shape);
}

}

#endif