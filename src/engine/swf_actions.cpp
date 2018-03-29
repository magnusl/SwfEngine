#include "swf_actions.h"
#include "swf_decode.h"
#include "swf_tags.h"
#include "swf_exception.h"
#include <assert.h>
#include <string>

using namespace std;

namespace swf_redux
{
namespace swf
{

void parse_placeobject(io::Source & source, PlaceObject_t & place)
{
    place.CharacterId	= source.get_u16();
	place.Depth			= source.get_u16();

	Decode<Matrix_Fixed2x3>(source, place.TransformationMatrix);
}

void parse_placeobject2(io::Source & source, PlaceObject2_t & place)
{
    source.assure_alignment();
	place.PlaceFlagHasClipActions		= source.get_bits(1);
	place.PlaceFlagHasClipDepth			= source.get_bits(1);
	place.PlaceFlagHasName				= source.get_bits(1);
	place.PlaceFlagHasRatio				= source.get_bits(1);
	place.PlaceFlagHasColorTransform	= source.get_bits(1);
	place.PlaceFlagHasMatrix			= source.get_bits(1);
	place.PlaceFlagHasCharacter			= source.get_bits(1);
	place.PlaceFlagMove					= source.get_bits(1);

	source.assure_alignment();

	place.Depth							= source.get_u16();
	if(place.PlaceFlagHasCharacter)	{
		place.CharacterId = source.get_u16();
	}
	if(place.PlaceFlagHasMatrix) {
		Decode<Matrix_Fixed2x3>(source, place.TransformationMatrix);
	}
	if(place.PlaceFlagHasColorTransform) {
		Decode(source, place.cxform);
	}
	place.Ratio		= place.PlaceFlagHasRatio ? source.get_u16() : 0;
	if(place.PlaceFlagHasRatio) {
		//place.Ratio = a_Source.GetU16();
	}
	if(place.PlaceFlagHasName) {
		std::string name = source.get_string();
        assert(false);
		//place.Name = stringTable.set(name);
	} else {
		place.Name = 0;
	}
	if (place.PlaceFlagHasClipDepth) {
		place.ClipDepth = source.get_u16();
	} else {
		place.ClipDepth = 0;
	}
}

void parse_placeobject3(io::Source & source, PlaceObject3_t & place)
{
	place.PlaceFlagHasClipActions		= source.get_bits(1);
	place.PlaceFlagHasClipDepth			= source.get_bits(1);
	place.PlaceFlagHasName				= source.get_bits(1);
	place.PlaceFlagHasRatio				= source.get_bits(1);
	place.PlaceFlagHasColorTransform	= source.get_bits(1);
	place.PlaceFlagHasMatrix			= source.get_bits(1);
	place.PlaceFlagHasCharacter			= source.get_bits(1);
	place.PlaceFlagMove					= source.get_bits(1);
		
	place.Reserved						= source.get_bits(1);
	place.PlaceFlagOpaqueBackground		= source.get_bits(1);
	place.PlaceFlagHasVisible			= source.get_bits(1);
	place.PlaceFlagHasImage				= source.get_bits(1);
	place.PlaceFlagHasClassName			= source.get_bits(1);
	place.PlaceFlagHasCacheAsBitmap		= source.get_bits(1);
	place.PlaceFlagHasBlendMode			= source.get_bits(1);
	place.PlaceFlagHasFilterList		= source.get_bits(1);
		
	place.Depth							= source.get_u16();

	if (place.PlaceFlagHasClassName ||
		(place.PlaceFlagHasImage && place.PlaceFlagHasCharacter))
	{
		string className = source.get_string();
		//place.ClassName = a_StringTable.set(className);
	}

	if(place.PlaceFlagHasCharacter)	{
		place.CharacterId = source.get_u16();
	}

	if(place.PlaceFlagHasMatrix) {
		Decode<Matrix_Fixed2x3>(source, place.TransformationMatrix);
	}
	if(place.PlaceFlagHasColorTransform) {
		Decode(source, place.cxform);
	}

	place.Ratio		= place.PlaceFlagHasRatio ? source.get_u16() : 0;
	if(place.PlaceFlagHasRatio) {
		place.Ratio = source.get_u16();
	}
	if(place.PlaceFlagHasName) {
		std::string name = source.get_string();
		//place.Name = a_StringTable.set(name);
	} else {
		place.Name = 0;
	}
	place.ClipDepth = place.PlaceFlagHasClipDepth ? source.get_u16() : 0;

	if (place.PlaceFlagHasFilterList) {
		uint8_t numFilter = source.get_u8();
		place.NumFilters = (numFilter > 8) ? 8 : numFilter; /**< we only care for the first 8 */
		for(size_t i = 0; i < numFilter; ++i)
		{
			if (i < 8) {
				Decode(source, place.Filters[i]);
			} else { /**< ignore remaining */
				Filter filter;
				Decode(source, filter);
			}
		}
	}

	if (place.PlaceFlagHasBlendMode) {
		place.BlendMode = source.get_u8();
	}
	if (place.PlaceFlagHasCacheAsBitmap) {
		place.BitmapCache = source.get_u8();
	}
	if (place.PlaceFlagHasVisible) {
		place.Visible = source.get_u8();
	}
	if (place.PlaceFlagOpaqueBackground) {
		Decode((io::Source &) source, place.BackgroundColor);
	}
	if (place.PlaceFlagHasClipActions) {
		throw std::runtime_error("TODO: CLIPACTIONS.");
	}
}

void parse_removeobject(io::Source & source, RemoveObject_t & remove)
{
	remove.CharacterId	= source.get_u16();
	remove.Depth		= source.get_u16();
}

void parse_removeobject2(io::Source & source, RemoveObject2_t & remove2)
{
	remove2.Depth = source.get_u16();
}

void parse_action(uint16_t tag_code, io::Source & source, Action & action)
{
    switch(tag_code) {
    case SwfTag_FrameLabel:
    case SwfTag_PlaceObject:
        action.type = Action::ePlaceObject;
        parse_placeobject(source, action.u.Place);
        break;
    case SwfTag_PlaceObject2:
    action.type = Action::ePlaceObject2;
        parse_placeobject2(source, action.u.Place2);
        break;
    case SwfTag_PlaceObject3:
        action.type = Action::ePlaceObject3;
        parse_placeobject3(source, action.u.Place3);
        break;
    case SwfTag_RemoveObject:
        action.type = Action::eRemoveObject;
        parse_removeobject(source, action.u.Remove);
        break;
    case SwfTag_RemoveObject2:
        action.type = Action::eRemoveObject;
        parse_removeobject(source, action.u.Remove);
        break;
    case SwfTag_SetBackgroundColor:
        break;
    default:
        throw ParseError("Invalid action.");
    }
}

}

}