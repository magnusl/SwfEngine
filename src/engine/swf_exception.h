#ifndef SWF_EXCEPTION_H
#define SWF_EXCEPTION_H

#include <stdexcept>

namespace swf_redux
{

class ParseError : public std::runtime_error
{
public:
    ParseError(const std::string & message) : std::runtime_error(message)
    {
    }

    ParseError(const char * message) : std::runtime_error(message)
    {
    }
};

} // namespace swf_redux

#endif