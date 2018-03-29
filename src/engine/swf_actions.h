#ifndef SWF_ACTIONS_H
#define SWF_ACTIONS_H

#include "swf_source.h"
#include "swf_types.h"

namespace swf_redux
{

namespace swf
{

/// Parses a PlaceObject tag
///
void parse_placeobject(io::Source &, PlaceObject_t &);

/// Parses a PlaceObject2 tag
///
void parse_placeobject2(io::Source &, PlaceObject2_t &);

/// Parses a PlaceObject3 tag
///
void parse_placeobject3(io::Source &, PlaceObject3_t &);

/// Parses a RemoveObject tag
///
void parse_removeobject(io::Source &, RemoveObject_t &);

/// Parses a RemoveObject2 tag.
///
void parse_removeobject2(io::Source &, RemoveObject2_t &);

struct Action
{
    typedef enum {
		eUnknownAction,
		eSetBackground,
		ePlaceObject,
		ePlaceObject2,
		ePlaceObject3,
		eRemoveObject,
		eRemoveObject2
	} ActionType_t;

    ActionType_t type;
        union {
        PlaceObject_t		Place;
        PlaceObject2_t		Place2;
        PlaceObject3_t		Place3;
        RemoveObject_t		Remove;
        RemoveObject2_t		Remove2;
	} u;
};

/// Parses a action from a source.
///
void parse_action(uint16_t tag_code, io::Source & source, Action & action);

}

}

#endif