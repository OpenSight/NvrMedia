/**
 * This file is part of stsw_proxy_source, which belongs to StreamSwitch
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
 * stsw_main.cc
 *      main entry file for ffmpeg_source
 * 
 * author: jamken
 * date: 2015-10-16
**/ 



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <stream_switch.h>

#include "stsw_stream_proxy.h"
#include "stsw_log.h"


///////////////////////////////////////////////////////////////
//Type


//////////////////////////////////////////////////////////////
//global variables



///////////////////////////////////////////////////////////////
//macro





///////////////////////////////////////////////////////////////
//functions
    



    
///////////////////////////////////////////////////////////////
//main entry    
int main(int argc, char *argv[])
{
    using namespace stream_switch;        
    int ret = 0;
    StreamProxySource * proxy = NULL;
    int pub_queue_size = STSW_PUBLISH_SOCKET_HWM;
    int sub_queue_size = STSW_SUBSCRIBE_SOCKET_HWM;   
    
    GlobalInit();
    
    //parse the cmd line
    ArgParser parser;
    ParseArgv(argc, argv, &parser); // parse the cmd line    
    
    //
    // init global logger
    if(parser.CheckOption(std::string("log-file"))){        
        //init the global logger
        std::string log_file = 
            parser.OptionValue("log-file", "");
        int log_size = 
            strtol(parser.OptionValue("log-size", "0").c_str(), NULL, 0);
        int rotate_num = 
            strtol(parser.OptionValue("log-rotate", "0").c_str(), NULL, 0);      
        int log_level = LOG_LEVEL_DEBUG;
        
        ret = InitGlobalLogger(log_file, log_size, rotate_num, log_level);
        if(ret){
            fprintf(stderr, "Init Logger failed, exit\n");
            goto exit_1;
        }        
    }


    
   
    //
    //init source
    
    proxy = StreamProxySource::Instance();   
      
    if(parser.CheckOption("pub-queue-size")){
        pub_queue_size = (int)strtol(parser.OptionValue("pub-queue-size", "60").c_str(), NULL, 0);
    }
    if(parser.CheckOption("sub-queue-size")){
        sub_queue_size = (int)strtol(parser.OptionValue("sub-queue-size", "120").c_str(), NULL, 0);
    }   
    
    ret = proxy->Init(
        parser.OptionValue("url", ""), 
        parser.OptionValue("stream-name", ""), 
        (int)strtol(parser.OptionValue("port", "0").c_str(), NULL, 0), 
        sub_queue_size, 
        pub_queue_size, 
        (int)strtol(parser.OptionValue("debug-flags", "0").c_str(), NULL, 0));
    if(ret){
        STDERR_LOG(stream_switch::LOG_LEVEL_ERR, 
                    "Proxy init error, exit\n");                 
   
        goto exit_2;       
    }
    
    
    //setup metadata
    ret = proxy->UpdateStreamMetaData(5000, NULL);
    if(ret){
        STDERR_LOG(stream_switch::LOG_LEVEL_ERR, 
                    "Proxy UpdateStreamMetaData error, exit\n");                 
        goto exit_3;
    }   
    
    //start proxy
    ret = proxy->Start();
    if(ret){
        goto exit_3;
    }

    //drive the proxy heartbeat    
    while(1){
        
        if(isGlobalInterrupt()){
            STDERR_LOG(stream_switch::LOG_LEVEL_INFO, 
                      "Receive Terminate Signal, exit\n");  
            ret = 0;    
            break;
        }

        ret = proxy->Hearbeat();
        if(ret){
            //error
            STDERR_LOG(stream_switch::LOG_LEVEL_ERR, 
                      "Proxy hearbeat error, exit\n");              
            break;
        }        
        usleep(100000);  //100 ms                 
    }
    
    
    //stop proxy
 
    proxy->Stop();    
    
exit_3:
    //uninit proxy
    proxy->Uninit();
    
    //uninstance
    StreamProxySource::Uninstance();  
    
exit_2:    
    //uninit logger
    UninitGlobalLogger();
exit_1:    
    
    //streamswitch library uninit
    GlobalUninit();
    
    return ret;
}

