#include "swf_dictionary.h"
#include "critical_section.h"
#include <map>
#include <iostream>

using namespace std;

namespace swf_redux
{

namespace swf
{

struct Dictionary::Impl
{
    /// maps a identifier to a type
    map<uint16_t, Dictionary::CharacterType>    type_map;
    /// movieclips
    map<uint16_t, shared_ptr<MovieClip> >       clips;
    map<uint16_t, shared_ptr<gfx::Shape> >      shapes;
    os::CriticalSection                         critical_section;
};

Dictionary::Dictionary()
{
    _impl = new Dictionary::Impl();
}

Dictionary::~Dictionary()
{
    delete _impl;
}

bool Dictionary::add_movieclip(uint16_t id, std::shared_ptr<MovieClip> clip)
{
    // enter the critial section before trying to access the dictionary.
    os::Lock lock(_impl->critical_section);
    // check if it already exists
    if (_impl->type_map.find(id) != _impl->type_map.end()) {
        return false;
    }
    // does not exist, so add it
    _impl->type_map[id] = Dictionary::CHAR_SPRITE;
    _impl->clips[id]    = clip;
    return true;
}

/// adds a shape to the dictionary
bool Dictionary::add_shape(uint16_t id, std::shared_ptr<gfx::Shape> shape)
{
    os::Lock lock(_impl->critical_section);
    if (_impl->type_map.find(id) != _impl->type_map.end()) {
        return false;
    }
    _impl->type_map[id] = Dictionary::CHAR_SHAPE;
    _impl->shapes[id]   = shape;
    
    return true;
}

/// returns a shape
std::shared_ptr<gfx::Shape> Dictionary::get_shape(uint16_t id)
{
    os::Lock lock(_impl->critical_section);
    map<uint16_t, shared_ptr<gfx::Shape> >::iterator it = _impl->shapes.find(id);
    if (it != _impl->shapes.end()) {
        return std::shared_ptr<gfx::Shape>();
    }
    return it->second;
}

}

} // namespace swf_redux