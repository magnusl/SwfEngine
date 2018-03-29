#ifndef SPRITE_JOB_H
#define SPRITE_JOB_H

#include "threadpool.h"
#include "tag_decoder.h"
#include "swf_dictionary.h"
#include <memory>

namespace swf_redux
{
 
/// Job for decoding a sprite definition
///
class SpriteDecodeJob : public os::IJob
{
public:
    SpriteDecodeJob(std::shared_ptr<TagDecoder>, swf::Dictionary & dictionary);
    virtual bool execute(void);

protected:
    SpriteDecodeJob(const SpriteDecodeJob &);
    SpriteDecodeJob & operator=(const SpriteDecodeJob &);

    std::shared_ptr<TagDecoder> _tag_data;
    swf::Dictionary & _dictionary;
};

}

#endif