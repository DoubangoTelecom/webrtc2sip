/* Copyright (C) 2012 Doubango Telecom <http://www.doubango.org>
*
* This file is part of Open Source 'webrtc2sip' project 
* <http://code.google.com/p/webrtc2sip/>
*
* 'webrtc2sip' is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*	
* 'webrtc2sip' is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*	
* You should have received a copy of the GNU General Public License
* along with 'webrtc2sip'.
*/
#if !defined(_MEDIAPROXY_CONFIG_H_)
#define _MEDIAPROXY_CONFIG_H_

#define WEBRTC2SIP_VERSION_MAJOR 2
#define WEBRTC2SIP_VERSION_MINOR 2
#define WEBRTC2SIP_VERSION_MICRO 0

#if defined(WIN32)|| defined(_WIN32) || defined(_WIN32_WCE)
#	define MP_UNDER_WINDOWS	1
#endif

#if MP_UNDER_WINDOWS
#	if !defined(_WIN32_WINNT)
#		define _WIN32_WINNT 0x0500
#	endif /* _WIN32_WINNT */
#endif /* MP_UNDER_WINDOWS */

#ifdef _MSC_VER
#	define _CRT_SECURE_NO_WARNINGS
#	define MP_INLINE	_inline
#else
#	define MP_INLINE	inline
#endif

// Default audio values
#define MP_AUDIO_CHANNELS_DEFAULT				1
#define MP_AUDIO_BITS_PER_SAMPLE_DEFAULT		16
#define MP_AUDIO_RATE_DEFAULT					8000
#define MP_AUDIO_PTIME_DEFAULT					20

#define MP_AUDIO_MIXER_VOL						0.8f
#define MP_AUDIO_MIXER_DIM						MPDimension_2D

#define MP_AUDIO_MAX_LATENCY					200 //ms
#define MP_AUDIO_RTP_TIMEOUT					2500 //ms

// Default video values
#define MP_VIDEO_WIDTH_DEFAULT					1280 // HD 720p
#define MP_VIDEO_HEIGHT_DEFAULT					720 // HD 720p
#define MP_VIDEO_FPS_DEFAULT					15

#define MP_VIDEO_RTP_TIMEOUT					2500 //ms
#define MP_VIDEO_MIXER_DIM						MPDimension_2D

#define MP_INVALID_ID	0

#include <stdlib.h>

#include "mp_common.h"

#endif /* _MEDIAPROXY_CONFIG_H_ */
