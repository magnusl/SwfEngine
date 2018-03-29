#ifndef SWF_PARSER_H
#define SWF_PARSER_H

#include "swf_source.h"
#include <memory>

namespace swf_redux
{

namespace swf
{

/// ShockWave Flash (SWF) parser.
///
class SWFParser
{
public:
    /// Parses the SWF from a source.
    bool parse(std::shared_ptr<io::Source> source);

protected:
    bool parse_tags(std::shared_ptr<io::Source> source);
};

}

} // namespace swf_redux


#endif