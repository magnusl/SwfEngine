#ifndef SWF_FRAME_H
#define SWF_FRAME_H

#include "swf_actions.h"
#include <vector>
#include <string>

namespace swf_redux
{

namespace swf
{

/// A frame containing one or more actions.
///
struct Frame
{
    // frame label
    std::string         frame_label;
    // frame actions
    std::vector<Action> actions;
};

/// A movieclip contains one or more frames
///
struct MovieClip
{
    std::vector<Frame>  frames;
};

}

}

#endif