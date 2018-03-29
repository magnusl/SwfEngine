#include "swf_parser.h"
#include "tag_decoder.h"
#include "swf_exception.h"
#include "swf_tags.h"
#include "shape_job.h"
#include "sprite_job.h"
#include "threadpool.h"
#include "sysinfo.h"
#include "swf_types.h"
#include "swf_decode.h"
#include <memory>
#include <iostream>
#include "swf_logger.h"

using namespace std;

namespace swf_redux
{

namespace swf
{

bool SWFParser::parse(std::shared_ptr<io::Source> source)
{
    swf_redux::Logger err_log(CHANNEL_PARSING, LOG_ERROR);
    swf_redux::Logger info_log(CHANNEL_PARSING, LOG_INFO);


    FileHeader hdr;
	try {
		// read the signature and version to determine if the file is compressed
		// or not.
		hdr.Signature[2] = source->get_u8();
		hdr.Signature[1] = source->get_u8();
		hdr.Signature[0] = source->get_u8();
		hdr.Version		 = source->get_u8();
		hdr.FileSize	 = source->get_u32();

        info_log << "SWF version=" << (uint32_t) hdr.Version << " size=" << hdr.FileSize << endl;

        std::shared_ptr<io::Source> swf_source;
        if (!memcmp(hdr.Signature, "SWF", 3)) { // uncompressed
			swf_source = source;
		} else if (!memcmp(hdr.Signature, "SWC", 3)) { // zlib compressed
			return false;
		} else if (!memcmp(hdr.Signature, "SWZ", 3)) { // LZMA compressed
			return false;
		} else {
			return false;
		}
		if (hdr.Version < 8) { // unsupported legacy format 
			return false;
		}	
		// decode the rest of the header
		Decode<FileHeader>(*swf_source, hdr);

        // version 8 or above all starts with a FileAttributes tag.
        TagDecoder tag(*swf_source);
        if (tag.tag_code() != SwfTag_FileAttributes) {
            return false;
        }
        uint8_t reserved		= tag.get_bits(1);
		uint8_t UseDirectBlit   = tag.get_bits(1);
		uint8_t UseGPU			= tag.get_bits(1);
		uint8_t HasMetadata		= tag.get_bits(1);
		uint8_t ActionScript3	= tag.get_bits(1);
		tag.get_bits(24);

        // now decode the tags
        return parse_tags(source);
    } catch(...) {
        err_log << "Caught exception while parsing SWF." << endl;
        return false;
    }
}

bool SWFParser::parse_tags(std::shared_ptr<io::Source> source)
{

    long num_jobs_submitted = 0;
    bool failure = false;
    os::ThreadPool pool(os::get_logical_processor_count());
    Dictionary dictionary;

    /// continue until we have reached the end of the file or
    /// a error occurs.
    while(!source->end_of_file() && !pool.error_encountered())
    {
        try {
            shared_ptr<TagDecoder> tag = make_shared<TagDecoder>(*source);
            switch(tag->tag_code())
            {
            case SwfTag_DefineShape:
            case SwfTag_DefineShape2:
            case SwfTag_DefineShape3:
            case SwfTag_DefineShape4:
                pool.submit_job(make_shared<ShapeDecodeJob>(tag, dictionary));
                break;
            case SwfTag_DefineSprite:
                pool.submit_job(make_shared<SpriteDecodeJob>(tag, dictionary));
                break;
            }
        } catch(swf_redux::ParseError &) {
            // parse error while reading tag.
            failure = true;
            break;
        }
    }
    if (pool.error_encountered() || failure) {
        // a error occurred while parsing the file,
        // abort without finishing the remaining jobs
        pool.abort();
    }
    // always wait for all the jobs to finish
    pool.wait();
    // return true if we didn't encounter a error
    return (!pool.error_encountered() && !failure);

}

}

}