#include "threadpool.h"
#include "tag_decoder.h"
#include "swf_dictionary.h"
#include <memory>

namespace swf_redux
{

/// Job for decoding a shape
///
class ShapeDecodeJob : public os::IJob
{
public:
    ShapeDecodeJob(std::shared_ptr<TagDecoder>, swf::Dictionary &);
    virtual bool execute(void);

protected:
    ShapeDecodeJob(const ShapeDecodeJob &);
    ShapeDecodeJob & operator=(const ShapeDecodeJob &);
    
private:
    std::shared_ptr<TagDecoder> _tag_data;
    swf::Dictionary &           _dictionary;
};

} // namespace swf_redux