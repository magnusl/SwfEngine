#ifndef SWF_DICTIONARY_H
#define SWF_DICTIONARY_H

#include "swf_frame.h"
#include "swf_shape.h"
#include <memory>

namespace swf_redux
{
namespace swf
{

/// A dictionary that stores the characters defined in a ShockWave Flash (SWF) file.
///
class Dictionary
{
public:
    /// Defines the different character types
    enum CharacterType {
        CHAR_SPRITE,
        CHAR_SHAPE
    };

    Dictionary();
    ~Dictionary();

    /// returns the type of the character identified by the identifier
    CharacterType get_type(uint16_t id);

    /// adds a movieclip to the dictionary
    bool add_movieclip(uint16_t id, std::shared_ptr<MovieClip> clip);
    /// returns a movieclip
    std::shared_ptr<MovieClip> get_movieclip(uint16_t id);

    /// adds a shape to the dictionary
    bool add_shape(uint16_t id, std::shared_ptr<gfx::Shape> shape);
    /// returns a shape
    std::shared_ptr<gfx::Shape> get_shape(uint16_t id);

protected:
    struct Impl;
    Impl * _impl;
};

}

}

#endif