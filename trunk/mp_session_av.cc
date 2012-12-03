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
#include "mp_session_av.h"

#include "MediaSessionMgr.h"

#include <assert.h>

namespace webrtc2sip {

MPSipSessionAV::MPSipSessionAV(CallSession** ppCallSessionToWrap, MPMediaType_t eMediaType)
: MPSipSession((SipSession **)ppCallSessionToWrap)
, m_eMediaType(eMediaType)
{
}

MPSipSessionAV::~MPSipSessionAV()
{
	if(m_oRtcpCallback){
		const_cast<CallSession*>(getWrappedCallSession())->setRtcpCallback(NULL, twrap_media_video);
	}

	TSK_DEBUG_INFO("MPSipSessionAV object destroyed");
}

void MPSipSessionAV::setState(MPSessionState_t eState)
{
	// call base
	MPSipSession::setState(eState);

	switch(eState){
		case MPSessionState_None:
		case MPSessionState_Connecting:
		default:
			{
				break;
			}
		case MPSessionState_Connected:
			{
				// now that the media session manage is up we can set the RTCP callback
				if(!m_oRtcpCallback && MediaSessionMgr::defaultsGetByPassEncoding() && MediaSessionMgr::defaultsGetByPassDecoding()){
					TSK_DEBUG_INFO("Media encoding/decoding is bypassed -> install RTCP callbacks for forwarding");
					m_oRtcpCallback = new MPSipSessionAVRtcpCallback(this);
					const_cast<CallSession*>(getWrappedCallSession())->setRtcpCallback(*m_oRtcpCallback, MPMediaType_GetNative(m_eMediaType));
				}
				break;
			}
		case MPSessionState_Terminated:
			{
				setSessionOpposite(NULL); // unRef()
				break;
			}
	}
}

int MPSipSessionAV::onRtcpEventCallback(const RtcpCallbackData* pcEvent)const
{
	if(pcEvent && m_oSessionOpposite){
		return const_cast<CallSession*>(m_oSessionOpposite->getWrappedCallSession())->sendRtcpEvent(pcEvent->getType(), MPMediaType_GetNative(m_eMediaType), pcEvent->getSSRC());
	}
	return 0;
}


} // namespace
