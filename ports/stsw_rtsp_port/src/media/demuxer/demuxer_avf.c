/**
 * This file is part of stsw_rtsp_port, which belongs to StreamSwitch
 * project. 
 * 
 * Copyright (C) 2015  OpenSight team (www.opensight.cn)
 * 
 * StreamSwitch is an extensible and scalable media stream server for 
 * multi-protocol environment. 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include <string.h>
#include <stdio.h>

#include "feng_utils.h"
#include "fnc_log.h"

#include "feng.h"

#include "media/demuxer_module.h"

#include <libavformat/avformat.h>

static const DemuxerInfo info = {
	"Avformat Demuxer",
	"avf",
	"LScube Team",
	"",
	"mov, nut, mkv, mxf", 
    0, 
};

typedef struct id_tag {
    const int id;
    const int pt;
    const char tag[11];
} id_tag;

//FIXME this should be simplified!
static const id_tag id_tags[] = {
   { CODEC_ID_MPEG1VIDEO, 32, "MPV" },
   { CODEC_ID_MPEG2VIDEO, 32, "MPV" },
   { CODEC_ID_H264, 98, "H264" },
   { CODEC_ID_MP2, 14, "MPA" },
   { CODEC_ID_MP3, 14, "MPA" },
   { CODEC_ID_VORBIS, 96, "VORBIS" },
   { CODEC_ID_THEORA, 96, "THEORA" },
   { CODEC_ID_SPEEX, 96, "SPEEX" },
   { CODEC_ID_AAC, 96, "AAC" },
   { CODEC_ID_MPEG4, 97, "MP4V-ES" },
   { CODEC_ID_H263, 96, "H263P" },
   { CODEC_ID_AMR_NB, 96, "AMR" },
   { CODEC_ID_PCM_MULAW, 0, "PCMU"},
   { CODEC_ID_PCM_ALAW, 8, "PCMA"},
   { CODEC_ID_NONE, 0, "NONE"}//XXX ...
};

typedef struct lavf_priv{
//    AVInputFormat *avif;
    AVFormatContext *avfc;
//    ByteIOContext *pb;
//    int audio_streams;
//    int video_streams;
    int64_t last_pts; //Use it or not?
} lavf_priv_t;

static const char *tag_from_id(int id)
{
    const id_tag *tags = id_tags;
    while (tags->id != CODEC_ID_NONE) {
        if (tags->id == id)
            return tags->tag;
        tags++;
    }
    return NULL;
}

static int pt_from_id(int id)
{
    const id_tag *tags = id_tags;
    while (tags->id != CODEC_ID_NONE) {
        if (tags->id == id)
            return tags->pt;
        tags++;
    }
    return 0;
}

#define PROBE_BUF_SIZE 2048

static int avf_probe(const char *filename)
{
    AVProbeData avpd = {
        .filename = filename
    };
    AVInputFormat *avif;
    uint8_t buffer[PROBE_BUF_SIZE];
    size_t rsize;

    FILE *fp = fopen(filename, "r");
    if ( !fp )
        return RESOURCE_DAMAGED;

    rsize = fread(&buffer, 1, sizeof(buffer), fp);
    fclose(fp);

    avpd.buf = &buffer[0];
    avpd.buf_size = rsize;

    av_register_all();

    if ( (avif = av_probe_input_format(&avpd, 1)) == NULL )
        return RESOURCE_DAMAGED;

    return RESOURCE_OK;
}

static double avf_timescaler (ATTR_UNUSED Resource *r, double res_time) {
    return res_time;
}

static int avf_init(Resource * r)
{
    AVFormatContext *avfc;
    //AVFormatParameters ap;
    lavf_priv_t *priv =  NULL;
    MediaProperties props;
    Track *track = NULL;
    TrackInfo trackinfo;
    int pt = 96;
    unsigned int i;
    int streamType = r->srv->srvconf.default_stream_type;



    //memset(&ap, 0, sizeof(AVFormatParameters));
    memset(&trackinfo, 0, sizeof(TrackInfo));
    priv = av_mallocz(sizeof(lavf_priv_t));

    avfc = NULL;
    //ap.prealloced_context = 1;

    //url_fopen(&priv->pb, r->info->mrl, URL_RDONLY);

    if (avformat_open_input(&avfc, r->info->mrl, NULL, NULL)) {
        fnc_log(FNC_LOG_DEBUG, "[avf] Cannot open %s", r->info->mrl);
        goto err_alloc;
    }
    avfc->flags |= AVFMT_FLAG_GENPTS;
    priv->avfc = avfc;

    for(i=0; i<avfc->nb_streams; i++) {
        AVStream *st= avfc->streams[i];
        AVCodecContext *codec= st->codec;
        if(codec->codec_type == AVMEDIA_TYPE_VIDEO) codec->flags2 |= CODEC_FLAG2_CHUNKS;
    }

    if(avformat_find_stream_info(avfc, NULL) < 0){
        fnc_log(FNC_LOG_DEBUG, "[avf] Cannot find streams in file %s",
                r->info->mrl);
        goto err_alloc;
    }

/* throw it in the sdp?
    if(avfc->title    [0])
    if(avfc->author   [0])
    if(avfc->copyright[0])
    if(avfc->comment  [0])
    if(avfc->album    [0])
    if(avfc->year        )
    if(avfc->track       )
    if(avfc->genre    [0]) */

    // make them pointers?
    strncpy(trackinfo.title, "TriNet NVR", 80);
    strncpy(trackinfo.author, "TriNet NVR", 80);

    r->info->duration = (double)avfc->duration /AV_TIME_BASE;
    r->model = MM_PULL;


    if(streamType != RAW_STREAM) {
        int videoAvailable = -1;

        r->private_data = priv;
        for(i=0; i<avfc->nb_streams; i++) {
            AVStream *st= avfc->streams[i];
            AVCodecContext *codec= st->codec;
            switch(codec->codec_type){
                case AVMEDIA_TYPE_AUDIO:
                    break;
                case AVMEDIA_TYPE_VIDEO:
                    videoAvailable = i;
                    break;
                case AVMEDIA_TYPE_DATA:
                case AVMEDIA_TYPE_UNKNOWN:
                case AVMEDIA_TYPE_SUBTITLE: //XXX import subtitle work!
                default:
                    fnc_log(FNC_LOG_DEBUG, "[avf] codec type unsupported");
                    break;
            }
        }
        if(videoAvailable != -1) {
            AVStream *st= avfc->streams[videoAvailable];
            AVCodecContext *codec= st->codec;
            trackinfo.id = 0;
            memset(&props, 0, sizeof(MediaProperties));
            props.clock_rate = 90000; //Default
            props.extradata = codec->extradata;
            props.extradata_len = codec->extradata_size;
            strncpy(props.encoding_name, "MP2P", 11);
            props.codec_id = 0;
            props.codec_sub_id = 0;
            props.payload_type = 96;
            props.media_type   = MP_video;
            props.bit_rate     = codec->bit_rate;
            props.frame_rate   = av_q2d(st->r_frame_rate);
            props.frame_type = FT_UNKONW;
            props.frame_duration     = (double)1 / props.frame_rate;
            props.AspectRatio  = codec->width *
                codec->sample_aspect_ratio.num /
                (float)(codec->height *
                        codec->sample_aspect_ratio.den);
            // addtrack must init the parser, the parser may need the
            // extradata
            snprintf(trackinfo.name,sizeof(trackinfo.name),"0");
            if (!(track = add_track(r, &trackinfo, &props))){
                goto err_alloc;
            }
        }
    }else{
        for(i=0; i<avfc->nb_streams; i++) {
            AVStream *st= avfc->streams[i];
            AVCodecContext *codec= st->codec;
            trackinfo.id = i;
            const char *id = tag_from_id(codec->codec_id);
    
            if (id) {
                memset(&props, 0, sizeof(MediaProperties));
                props.clock_rate = 90000; //Default
                props.extradata = codec->extradata;
                props.extradata_len = codec->extradata_size;
                strncpy(props.encoding_name, id, 11);
                props.codec_id = codec->codec_id;
                props.codec_sub_id = codec->codec_id;
                props.payload_type = pt_from_id(codec->codec_id);
                props.frame_type = FT_UNKONW;
                if (props.payload_type == 96){
                    
                    props.payload_type = pt++;
                }else if (props.payload_type > 96) {
                    if(pt <= props.payload_type ) {
                        pt = props.payload_type + 1;
                    }
                }
                fnc_log(FNC_LOG_DEBUG, "[avf] Parsing AVStream %s",
                        props.encoding_name);
            } else {
                fnc_log(FNC_LOG_DEBUG, "[avf] Cannot map stream id %d", i);
                continue;
            }
            switch(codec->codec_type){
                case AVMEDIA_TYPE_AUDIO:
                    props.media_type     = MP_audio;
                    // Some properties, add more?
                    props.bit_rate       = codec->bit_rate;
                    props.audio_channels = codec->channels;
                    // Make props an int...
                    props.sample_rate    = codec->sample_rate;
                    props.frame_duration       = (double)1 / codec->sample_rate;
                    props.bit_per_sample   = codec->bits_per_coded_sample;
                    snprintf(trackinfo.name, sizeof(trackinfo.name), "%d", i);
                    if (!(track = add_track(r, &trackinfo, &props)))
                        goto err_alloc;
                break;
                case AVMEDIA_TYPE_VIDEO:
                    props.media_type   = MP_video;
                    props.bit_rate     = codec->bit_rate;
                    props.frame_rate   = av_q2d(st->r_frame_rate);
                    props.frame_duration     = (double)1 / props.frame_rate;
                    props.AspectRatio  = codec->width *
                                          codec->sample_aspect_ratio.num /
                                          (float)(codec->height *
                                                  codec->sample_aspect_ratio.den);
                    // addtrack must init the parser, the parser may need the
                    // extradata
                    snprintf(trackinfo.name,sizeof(trackinfo.name),"%d",i);
                    if (!(track = add_track(r, &trackinfo, &props)))
                goto err_alloc;
                break;
                case AVMEDIA_TYPE_DATA:
                case AVMEDIA_TYPE_UNKNOWN:
                case AVMEDIA_TYPE_SUBTITLE: //XXX import subtitle work!
                default:
                    fnc_log(FNC_LOG_DEBUG, "[avf] codec type unsupported");
                break;
            }
        }
    
    }



    

    if (track) {

        fnc_log(FNC_LOG_DEBUG, "[avf] duration %f", r->info->duration);
        r->private_data = priv;
        r->timescaler = avf_timescaler;
        return RESOURCE_OK;
    }


err_alloc:


    if (priv) {
        if (priv->avfc) {
            avformat_close_input(&(priv->avfc));
            priv->avfc = NULL;
        }
        av_freep(&priv);
        r->private_data = NULL;
    }

    return ERR_PARSE;
}

static int avf_read_packet(Resource * r)
{
    int ret = RESOURCE_OK;
    TrackList tr_it;
    AVPacket pkt;
    AVStream *stream;
    lavf_priv_t *priv = r->private_data;
    int streamType = r->srv->srvconf.default_stream_type;

// get a packet
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    if(av_read_frame(priv->avfc, &pkt) < 0)
        return RESOURCE_EOF; //FIXME
    for (tr_it = g_list_first(r->tracks);
         tr_it !=NULL;
         tr_it = g_list_next(tr_it)) {
        Track *tr = (Track*)tr_it->data;
        if (pkt.stream_index == tr->info->id ||
            streamType != RAW_STREAM) {
            if(streamType != RAW_STREAM) {
                tr->info->id = pkt.stream_index;
            }
// push it to the framer
            stream = priv->avfc->streams[tr->info->id];
            fnc_log(FNC_LOG_VERBOSE, "[avf] Parsing track %s",
                    tr->info->name);
            if(pkt.dts != AV_NOPTS_VALUE) {
                tr->properties.dts = r->timescaler (r,
                    pkt.dts * av_q2d(stream->time_base));
#ifdef TRISOS
		   if(tr->properties.dts < 0){
               av_free_packet(&pkt);
               return RESOURCE_EOF;
           }
		   else{
#endif
                fnc_log(FNC_LOG_VERBOSE,
                        "[avf] delivery timestamp %f",
                        tr->properties.dts);
#ifdef TRISOS
		   }
#endif
            } else {
                fnc_log(FNC_LOG_VERBOSE,
                        "[avf] missing delivery timestamp");
#ifdef TRISOS
                av_free_packet(&pkt);
                return RESOURCE_NOT_PARSEABLE;
#endif
            }

            if(pkt.pts != AV_NOPTS_VALUE) {
                tr->properties.pts = r->timescaler (r,
                    pkt.pts * av_q2d(stream->time_base));
#ifdef TRISOS
			if(tr->properties.pts < 0){
                av_free_packet(&pkt);
                return RESOURCE_EOF;
		   	}
		   else{
#endif
                fnc_log(FNC_LOG_VERBOSE,
                        "[avf] presentation timestamp %f",
                        tr->properties.pts);
#ifdef TRISOS
		   }
#endif
            } else {
                fnc_log(FNC_LOG_VERBOSE, "[avf] missing presentation timestamp, replace with dts");
#ifdef TRISOS
                tr->properties.pts = r->timescaler (r,
                    pkt.dts * av_q2d(stream->time_base));

                if(tr->properties.pts < 0){
                    av_free_packet(&pkt);
                    return RESOURCE_EOF;
                }else{

                    fnc_log(FNC_LOG_VERBOSE,
                            "[avf] presentation timestamp %f",
                            tr->properties.pts);

                }
#endif
            }

            if (pkt.duration) {
                tr->properties.frame_duration = pkt.duration *
                    av_q2d(stream->time_base);
            } else { // welcome to the wonderland ehm, hackland...
                switch (stream->codec->codec_id) {
                    case CODEC_ID_MP2:
                    case CODEC_ID_MP3:
                        tr->properties.frame_duration = 1152.0/
                                tr->properties.sample_rate;
                        break;
                    default: break;
                }
            }

            fnc_log(FNC_LOG_VERBOSE, "[avf] packet duration %f",
                tr->properties.frame_duration);

            ret = tr->parser->parse(tr, pkt.data, pkt.size);
            break;
        }
    }

    av_free_packet(&pkt);

    return ret;
}

static int avf_seek(Resource * r, double time_sec)
{
    int flags = 0;
    int64_t time_msec = time_sec * AV_TIME_BASE;
    fnc_log(FNC_LOG_DEBUG, "Seeking to %f", time_sec);
    AVFormatContext *fc = ((lavf_priv_t *)r->private_data)->avfc;
    if (fc->start_time != AV_NOPTS_VALUE)
        time_msec += fc->start_time;
    if (time_msec < 0) flags = AVSEEK_FLAG_BACKWARD;
#ifdef TRISOS
    r->lastTimestamp = time_sec;
#endif

    return av_seek_frame(fc, -1, time_msec, flags);
}

static void avf_uninit(gpointer rgen)
{
    Resource *r = rgen;
    lavf_priv_t* priv = r->private_data;

// avf stuff
    if (priv) {
        if (priv->avfc) {
            avformat_close_input(&(priv->avfc));
            priv->avfc = NULL;
        }
        av_freep(&priv);
        r->private_data = NULL;
    }
}

#define avf_start NULL
#define avf_pause NULL;



FNC_LIB_DEMUXER(avf);

