/*
 * =====================================================================================
 *
 *       Filename:  sndf.cpp
 *
 *    Description:  :
 *
 *        Version:  1.0
 *        Created:  15.10.2017 17:39:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <wpn114/audio/backend/sndfile_support.hpp>

//template<typename T>
//T wpn114::audio::sndbuf16_t::load_soundfile(const std::string& path_to_soundfile)
static int wpn114::audio::load_soundfile(sndbuf_t& buffer, const std::string& path_to_soundfile)
{
    SNDFILE*    infile;
    SF_INFO     sfinfo;
    int         readcount;

    std::memset(&sfinfo, 0, sizeof(sfinfo));

    if(!(infile = sf_open (path_to_soundfile.c_str(), SFM_READ, &sfinfo)))
    {
        std::cerr << "could not open soundfile, aborting..\n";
        return -1;
    }

    buffer.data = (float*) malloc(sfinfo.frames * sfinfo.channels * sizeof(float));
    // remember that sndbuffers_t are interleaved

    // puts contents into buffer
    sf_count_t frames_read  = sf_read_float(infile, buffer.data, sfinfo.frames);

    buffer.num_channels     = sfinfo.channels;
    buffer.num_frames       = sfinfo.frames;
    buffer.num_samples      = (float) sfinfo.frames / sfinfo.channels;
    buffer.sample_rate      = sfinfo.samplerate;

    return 0;
}
