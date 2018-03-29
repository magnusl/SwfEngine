#ifndef _SWF_OBJTYPES_H_
#define _SWF_OBJTYPES_H_

/// @file   swf_objtypes.h
///
/// Defines the object type identifier for all
/// the embedded ActionScript objects.

namespace swf_redux
{

enum ObjectType
{
    TYPE_OBJECT = 0,    // ActionScript 3 object base.
    TYPE_SIGNED,
    TYPE_UNSIGNED,
    TYPE_BOOLEAN,
    TYPE_STRING,
    TYPE_ARRAY
};

} // namespace swf_redux

#endif