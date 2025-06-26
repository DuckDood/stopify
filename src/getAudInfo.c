#include <libavformat/avformat.h>
#include <libavutil/dict.h>

// google ai :((
// i did edit lil bit tho
// idk
// dont vibe code
char* artistName(char*filename) {
    AVFormatContext *format_context = NULL;
    AVDictionaryEntry *tag = NULL;

    // Register all formats and codecs (for older FFmpeg versions)
    // av_register_all(); 

    // Open the input file
    if (avformat_open_input(&format_context, filename, NULL, NULL) < 0) {
        return "";
    }

    // Retrieve stream information
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        avformat_close_input(&format_context);
        return "";
    }

    // Print global metadata
    while ((tag = av_dict_get(format_context->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
		if(!strcmp(tag->key, "artist"))
        return tag->value;
    }

    return "";
}
