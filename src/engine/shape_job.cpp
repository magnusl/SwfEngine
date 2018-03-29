#include "shape_job.h"
#include <iostream>
#include "swf_define.h"
#include "swf_shape.h"
#include "swf_logger.h"

using namespace std;

namespace swf_redux
{

ShapeDecodeJob::ShapeDecodeJob(std::shared_ptr<TagDecoder> tag, 
swf::Dictionary & dictionary) :
_tag_data(tag),
_dictionary(dictionary)
{
}

///
///
bool ShapeDecodeJob::execute(void)
{
    if (!_tag_data) {
        return false;
    }

    Logger info_log(CHANNEL_PARSING, LOG_INFO);
    Logger err_log(CHANNEL_PARSING, LOG_ERROR);
    
    swf::DefinedShape4 flash_shape;
    if (!swf::parse_defineshape4(*_tag_data, flash_shape)) {
        // return false to indicate that the job failed.
        err_log << "Failed parse ShockWave Flash shape." << endl;
        return false;
    }
    std::shared_ptr<gfx::Shape> shape = gfx::create_shape(flash_shape);
    if (!shape) {
        err_log << "Failed to create shape." << endl;
        return false;
    }
    if (!_dictionary.add_shape(flash_shape.ShapeId, shape)) {
        err_log << "Failed to add shape to dictionary." << endl;
        return false;
    }
    // return success
    return true;
}

}