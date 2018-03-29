#ifndef SWF_LOGGER_H
#define SWF_LOGGER_H

#include "swf_channel.h"

namespace swf_redux
{

enum LogLevel {
    LOG_QUIET,
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG,
    LOG_VERBOSE
};

class Logger
{
public:
    Logger(ChannelId id, LogLevel level = LOG_VERBOSE) : _level(level),
    _current_level(LOG_VERBOSE),
    _id(id)
    {
        // select the right output stream based on the channel id
    }

    template<class T>
    Logger & operator<<(const T & value)
    {
        if (_level <= _current_level) {
            std::cout << value;   
        }
        return *this;
    }

    typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
    typedef CoutType& (*StandardEndLine)(CoutType&);

    Logger & operator<<(StandardEndLine manip)
    {
        if (_level <= _current_level) {
            manip(std::cout);
        }
        return *this;
    }

protected:
    LogLevel    _level;
    LogLevel    _current_level;
    ChannelId   _id;
};

}

#endif