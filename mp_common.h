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
#include "mp_object.h"

#include "Common.h"

#define MP_CAT_(A, B) A ## B
#define MP_CAT(A, B) MP_CAT_(A, B)
#define MP_STRING_(A) #A
#define MP_STRING(A) MP_STRING_(A)

typedef int32_t MPNetFd;
#define MPNetFd_IsValid(self)	((self) > 0)
#define kMPNetFdInvalid			-1

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

typedef enum MPDbType_e
{
	MPDbType_None = 0x00,
	MPDbType_File = (0x01 << 0),
	MPDbType_Server =  (0x01 << 1),

	MPDbType_SQLite = (MPDbType_File | (0x01 << 2)),
	MPDbType_MySql = (MPDbType_Server | (0x01 << 3)),
	MPDbType_Oracle = (MPDbType_Server | (0x01 << 4)),
	MPDbType_SqlServer = (MPDbType_Server | (0x01 << 5))
}
MPDbType_t;

typedef enum MPNetTransporType_e
{
	MPNetTransporType_None,
	MPNetTransporType_TCP,
	MPNetTransporType_TLS
}
MPNetTransporType_t;

static bool MPNetTransporType_isStream(MPNetTransporType_t eType)
{
	switch(eType)
	{
	case MPNetTransporType_TCP:
	case MPNetTransporType_TLS:
		return true;
	default:
		return false;
	}
}

static MPSessionState_t MPPeerState_getSessionState(MPPeerState_t eState)
{
	switch(eState)
	{
		case MPPeerState_Connecting: return MPSessionState_Connecting;
		case MPPeerState_Connected: return MPSessionState_Connected;
		case MPSessionState_Terminated: return MPSessionState_Terminated;
		default: return MPSessionState_None;
	}
}

//
//	MPData
//
class MPData : public MPObject
{
public:
	MPData(void** ppData, size_t nDataSize, bool bTakeOwnership = true)
		: m_pDataPtr(NULL)
		, m_nDataSize(0)
		, m_bHaveOwnership(bTakeOwnership)
	{
		if(ppData)
		{
			m_pDataPtr = *ppData;
			m_nDataSize = nDataSize;
			if(m_bHaveOwnership)
			{
				*ppData = NULL;
			}
		}
	}
	virtual ~MPData()
	{
		if(m_bHaveOwnership)
		{
			if(m_pDataPtr)
			{
				free(m_pDataPtr);
			}
		}
	}
	virtual MP_INLINE const char* getObjectId() { return "MPData"; }
	virtual MP_INLINE const void* getDataPtr(){ return m_pDataPtr; }
	virtual MP_INLINE size_t getDataSize(){ return m_nDataSize; }

private:
	void* m_pDataPtr;
	size_t m_nDataSize;
	bool m_bHaveOwnership;
};

} // namespace
#endif /* _MEDIAPROXY_COMMON_H_ */
