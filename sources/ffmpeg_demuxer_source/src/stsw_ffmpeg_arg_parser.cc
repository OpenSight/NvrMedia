/**
 * This file is part of libstreamswtich, which belongs to StreamSwitch
 * project. 
 * 
 * Copyright (C) 2014  OpenSight (www.opensight.cn)
 * 
 * StreamSwitch is an extensible and scalable media stream server for 
 * multi-protocol environment. 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/**
 * stsw_ffmpeg_arg_parser.cc
 *      FFmpegArgParser class implementation file, FFmpegArgParser is 
 * the an arg parser for ffmpeg_source application. FFmpegArgParser 
 * extends the ArgParser to parse the ffmpeg options which start by 
 * "--ffmpeg-". 
 * 
 * author: OpenSight Team
 * date: 2015-10-17
**/ 

#include "stsw_ffmpeg_arg_parser.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>


#define FFMPEG_OPT_PRE  "--ffmpeg-"
    
    
FFmpegArgParser::FFmpegArgParser()

{

}


FFmpegArgParser::~FFmpegArgParser()
{
    
}


void FFmpegArgParser::RegisterSourceOptions()
{
    ArgParser::RegisterSourceOptions();
    
    //register the other options
    RegisterOption("io-timeout", 0, 
                   OPTION_FLAG_WITH_ARG | OPTION_FLAG_LONG, "MSEC",
                   "timeout (in millisec) for IO operation. Default is 10000 ms", NULL, NULL);
    RegisterOption("local-max-gap", 0, 
                   OPTION_FLAG_WITH_ARG | OPTION_FLAG_LONG, "SEC",
                   "the max time gap (in sec) between the local and the received live frame. "
                   "If the gap is over this limitation, this source would exit with error. "
                   "Defualt is 20 sec", NULL, NULL);
                   
    RegisterOption("native-frame-rate", 0, 0, NULL,
                   "ffmpeg_demuxer_source would read input at the native frame rate of the default stream. "
                   "Mainly used to simulate a live stream from a non-live input (like a media file). "
                   "Should not be used with the actual live input streams (where it can cause packet loss). ",
                    NULL, NULL);
                   
    RegisterOption("ffmpeg-[NAME]", 0, 
                   OPTION_FLAG_WITH_ARG, "VALUE",
                   "user can used --ffmpeg-[optioan]=[value] form to pass the options to the ffmpeg library. "
                   "ffmpeg_demuxer_source parse the options which start with \"ffmpeg-\", "
                   "and pass them to ffmpeg demuxing context. "
                   "The option name would be extracted from the string after \"ffmpeg-\", the value would be set to VALUE." ,
                   NULL, NULL);
                   
    RegisterOption("url", 'u', OPTION_FLAG_REQUIRED | OPTION_FLAG_WITH_ARG,
                   "URL", 
                   "the input file path or the network URL "
                   "which the source read media data from by ffmpeg demuxing library. "
                   "Should be a vaild input for ffmpeg's libavformat", NULL, NULL);  


    RegisterOption("debug-flags", 'd', 
                    OPTION_FLAG_LONG | OPTION_FLAG_WITH_ARG,  "FLAG", 
                    "debug flag for stream_switch core library. "
                    "Default is 0, means no debug dump" , 
                    NULL, NULL);      
}
    

bool FFmpegArgParser::ParseUnknown(const char * unknown_arg)
{
    //just ignore, user can override this function to print error message and exit
    if(unknown_arg == NULL){
        return false;
    }
    
    if(strncmp(unknown_arg, FFMPEG_OPT_PRE, strlen(FFMPEG_OPT_PRE)) == 0){
        std::string ffmpeg_option = 
            std::string(unknown_arg + strlen(FFMPEG_OPT_PRE));
        if(ffmpeg_option.find('=') == std::string::npos){
            ffmpeg_option.append("=1");
        }
        if(ffmpeg_options_.size() != 0){
            ffmpeg_options_.append(",");
        
        }
        ffmpeg_options_.append(ffmpeg_option);  
        return true;
    }
    
    return false;
}





