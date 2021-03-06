#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
version = 0.1.0

AC_PREREQ([2.63])
AC_INIT([streamswitch], [0.1.0], [zqwangjk@sina.com])

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])


AM_INIT_AUTOMAKE([tar-ustar])
AM_MAINTAINER_MODE
AM_SILENT_RULES


LT_PREREQ([2.2])
AC_PROG_LIBTOOL

AC_CONFIG_SRCDIR([libstreamswitch/include/stream_switch.h])
# AC_CONFIG_HEADERS([config.h])

# With feature list
AC_ARG_WITH([rtsp-source],
[AS_HELP_STRING([--with-rtsp-source],
[would contains the stsw_rtsp_source program @<:@default=yes@:>@])],
[rtsp_source=${withval}], [rtsp_source=yes])

AM_CONDITIONAL([INCLUDE_RTSP_SOURCE], [test "x$rtsp_source" = xyes])


AC_ARG_WITH([rtmp-source], 
[AS_HELP_STRING([--with-rtmp-source], 
[would contains the stsw_rtmp_source program @<:@default=no@:>@])], 
[rtmp_source=${withval}], [rtmp_source=no]) 

AM_CONDITIONAL([INCLUDE_RTMP_SOURCE], [test "x$rtmp_source" = xyes])


AC_ARG_WITH([proxy-source], 
[AS_HELP_STRING([--with-proxy-source], 
[would contains the stsw_proxy_source program @<:@default=yes@:>@])], 
[proxy_source=${withval}], [proxy_source=yes]) 

AM_CONDITIONAL([INCLUDE_PROXY_SOURCE], [test "x$proxy_source" = xyes])


AC_ARG_WITH([ffmpeg-demuxer-source], 
[AS_HELP_STRING([--with-ffmpeg-demuxer-source], 
[would contains the ffmpeg_demuxer_source program @<:@default=yes@:>@])], 
[ffmpeg_demuxer_source=${withval}], [ffmpeg_demuxer_source=yes]) 

AM_CONDITIONAL([INCLUDE_FFMPEG_DEMUXER_SOURCE], [test "x$ffmpeg_demuxer_source" = xyes])



AC_ARG_WITH([ffmpeg-sender], 
[AS_HELP_STRING([--with-ffmpeg-sender], 
[would contains the ffmpeg_sender program @<:@default=yes@:>@])], 
[ffmpeg_sender=${withval}], [ffmpeg_sender=yes]) 

AM_CONDITIONAL([INCLUDE_FFMPEG_SENDER], [test "x$ffmpeg_sender" = xyes])


AC_ARG_WITH([rtsp-port], 
[AS_HELP_STRING([--with-rtsp-port], 
[would contains the stsw_rtsp_port program @<:@default=yes@:>@])], 
[rtsp_port=${withval}], [rtsp_port=yes])

AM_CONDITIONAL([INCLUDE_RTSP_PORT], [test "x$rtsp_port" = xyes])




# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
# AC_PROG_RANLIB
AM_PROG_CC_C_O

# Set some default features required by streamswitch code.
CPPFLAGS="-D_REENTRANT -D_THREAD_SAFE $CPPFLAGS"
CFLAGS="-pthread $CFLAGS"
CXXFLAGS="-pthread $CXXFLAGS"


# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])

export PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/lib64/pkgconfig:/lib/pkgconfig:/lib64/pkgconfig:/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

PKG_CHECK_MODULES(zeromq, [libczmq >= 3.0.0 libzmq >= 4.0.0])

PKG_CHECK_MODULES(protobuf, [protobuf >= 2.0.0])


# Checks for stsw_rtmp_source
if test "x$rtmp_source" = xyes; then
  PKG_CHECK_MODULES(librtmp, [librtmp >= v2.3])
fi


# Checks for stsw_rtsp_port
if test "x$rtsp_port" = xyes; then
  AC_CHECK_HEADERS([ev.h], [have_libev=yes], [have_libev=no])
  AS_IF([test "x$have_libev" = "xno"], [AC_MSG_ERROR([libev not found, stsw_rtsp_port requires libev, libev must be installed into default position])])
  
  PKG_CHECK_MODULES(libnetembryo, [libnetembryo >= 0.1.0])  
  PKG_CHECK_MODULES(glib, [glib-2.0 >= 2.16 gthread-2.0])  
fi



# Checks for ffmpeg_demuxer_source & ffmpeg_sender
if test "x$ffmpeg_demuxer_source" = xyes -o "x$ffmpeg_sender" = xyes; then
  PKG_CHECK_MODULES(libavformat, [libavformat >= 56.40.0])
  PKG_CHECK_MODULES(libavutil, [libavutil >= 54.31.100])    
fi

# Checks for ffmpeg_sender
if test "x$ffmpeg_sender" = xyes; then
  PKG_CHECK_MODULES(libavcodec, [libavcodec >= 56.60.100])
  PKG_CHECK_MODULES(libswresample, [libswresample >= 1.2.101])   
  PKG_CHECK_MODULES(libffmpeg_ivr, [libffmpeg_ivr], [AC_DEFINE(HAVE_LIBFFMPEG_IVR, 1)], [AC_MSG_WARN(libffmpeg_ivr not found)])  
fi


# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/time.h unistd.h czmq.h zmq.h libffmpeg_ivr.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([dup2 gettimeofday memset strtol])

# generate Makefiles 
AC_CONFIG_FILES([Makefile \
                 libstreamswitch/Makefile \
                 libstreamswitch/samples/Makefile \
                 libstreamswitch/libstreamswitch.pc \
                 sources/stsw_rtsp_source/Makefile \
                 sources/stsw_rtmp_source/Makefile \
                 sources/stsw_proxy_source/Makefile \
                 sources/ffmpeg_demuxer_source/Makefile \
                 senders/ffmpeg_sender/Makefile \
                 ports/stsw_rtsp_port/Makefile])

# Generate other makefiles

if test "x$rtsp_source" = xyes; then
  AC_MSG_NOTICE(Create Makefiles for LIVE555)
  #sub-shell
  (cd $srcdir/sources/stsw_rtsp_source/live/; ./genMakefiles.stream_switch stream-switch)
fi

AC_OUTPUT



AC_MSG_NOTICE([Configure successfully!

StreamSwitch version ................. : $PACKAGE_VERSION
StreamSwitch Install prefix .......... : $prefix
StreamSwitch exec prefix ............. : $exec_prefix
StreamSwitch library path ............ : $libdir
StreamSwitch include path ............ : $includedir
StreamSwitch program path ............ : $bindir

stsw_rtsp_source included............. : $rtsp_source
stsw_rtmp_source included ............ : $rtmp_source
stsw_proxy_source included ........... : $proxy_source
ffmpeg_demuxer_source included ....... : $ffmpeg_demuxer_source
ffmpeg_sender included ............... : $ffmpeg_sender
stsw_rtsp_port included .............. : $rtsp_port




 'make' will now compile StreamSwitch and 'su -c make install' will install it.
 Note: On non-Linux systems you might need to use 'gmake' instead of 'make'.
])