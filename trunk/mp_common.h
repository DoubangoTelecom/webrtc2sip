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
#if !defined(_MEDIAPROXY_COMMON_H_)
#define _MEDIAPROXY_COMMON_H_

#include "mp_config.h"

#include "Common.h"

namespace webrtc2sip {

typedef enum MPMediaType_e
{
	MPMediaType_None = 0x00,
	MPMediaType_Audio = (0x01<<0),
	MPMediaType_Video = (0x01<<1),
	MPMediaType_AudioVideo = (MPMediaType_Audio | MPMediaType_Video),

	MPMediaType_All = 0xFF,
}
MPMediaType_t;

static enum twrap_media_type_e MPMediaType_GetNative(MPMediaType_t eType)
{
	enum twrap_media_type_e eNativeType = twrap_media_none;

	if((eType & MPMediaType_Audio)) eNativeType = (enum twrap_media_type_e)(eNativeType | twrap_media_audio);
	if((eType & MPMediaType_Video)) eNativeType = (enum twrap_media_type_e)(eNativeType | twrap_media_video);

	return eNativeType;
}

typedef enum MPPeerState_e
{
	MPPeerState_None,
	MPPeerState_Connecting,
	MPPeerState_Connected,
	MPPeerState_Terminated
}
MPPeerState_t;

typedef enum MPSessionState_e
{
	MPSessionState_None,
	MPSessionState_Connecting,
	MPSessionState_Connected,
	MPSessionState_Terminated
}
MPSessionState_t;

static MPSessionState_t MPPeerState_GetSessionState(MPPeerState_t eState)
{
	switch(eState){
		case MPPeerState_Connecting: return MPSessionState_Connecting;
		case MPPeerState_Connected: return MPSessionState_Connected;
		case MPSessionState_Terminated: return MPSessionState_Terminated;
		default: return MPSessionState_None;
	}
}

} // namespace
#endif /* _MEDIAPROXY_COMMON_H_ */
