#include "sprite_job.h"
#include <iostream>
#include "swf_logger.h"
#include "swf_tags.h"
#include "swf_actions.h"
#include "swf_frame.h"

using namespace std;

namespace swf_redux
{

SpriteDecodeJob::SpriteDecodeJob(std::shared_ptr<TagDecoder> tag, swf::Dictionary & dictionary) :
_tag_data(tag),
_dictionary(dictionary)
{
}

bool SpriteDecodeJob::execute(void)
{
    uint16_t id             = _tag_data->get_u16();
	uint16_t frame_count    = _tag_data->get_u16();
    uint16_t current_frame  = 0;

    // create logging instances to use.
    Logger info_log(CHANNEL_PARSING, LOG_INFO);
    Logger err_log(CHANNEL_PARSING, LOG_ERROR);


    shared_ptr<swf::MovieClip> clip = make_shared<swf::MovieClip>();
    clip->frames.resize(frame_count);

    try {
        // continue until all the frames has been parsed.
        while(current_frame < frame_count)
        {
            // don't invoke copy-constructor
            TagDecoder tag((io::Source &)*_tag_data);
            switch(tag.tag_code())
            {
            case SwfTag_ShowFrame:
                ++current_frame;
                break;
            case SwfTag_PlaceObject:
		    case SwfTag_PlaceObject2:
		    case SwfTag_PlaceObject3:
		    case SwfTag_RemoveObject:
		    case SwfTag_RemoveObject2:
		    case SwfTag_StartSound:
		    case SwfTag_StartSound2:
		    case SwfTag_FrameLabel:
		    case SwfTag_SoundStreamHead:
		    case SwfTag_SoundStreamHead2:
		    case SwfTag_SoundStreamBlock:
                {
                    swf::Action action;
                    swf::parse_action(tag.tag_code(), (io::Source &) tag, action);   // parse the action from the tag.
                    clip->frames[current_frame].actions.push_back(action);    // and add it to the frame.
                }
                break;
            case SwfTag_End:
                err_log << "Unexpected SwfTag_End tag." << endl;
                return false;
            default:
                // this tag is not allowed in a sprite
                err_log << "Tag " << tag.tag_code() << " isn't allowed in a sprite." << endl;
                return false;
            }
        }
    } catch(...) {
        err_log << "Caught a exception while parsing sprite." << endl;
        return false;
    }
    // the last tag must be a 'SwfTag_End' tag.
    TagDecoder tag((io::Source &)*_tag_data);
    if (tag.tag_code() != SwfTag_End) {
        err_log << "Expected a SwfTag_End at the end of the sprite" << endl;
        return false;
    }
    // add the movieclip to the dictionary with the id as key.
    return _dictionary.add_movieclip(id, clip);
}

} // namespace swf_redux
