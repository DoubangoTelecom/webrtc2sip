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
#if !defined(_MEDIAPROXY_PEER_H_)
#define _MEDIAPROXY_PEER_H_

#include "mp_config.h"
#include "mp_object.h"
#include "mp_session_av.h"

namespace webrtc2sip {

class MPPeer : public MPObject
{
public:
	MPPeer(MPObjectWrapper<MPSipSessionAV*> oCallSessionLeft);
	virtual ~MPPeer();
	MP_INLINE virtual const char* getObjectId() { return "MPPeer"; }

	MP_INLINE uint64_t getId(){ return m_nId; }
	MP_INLINE MPMediaType_t getMediaType(){ return m_eMediaType; }
	MP_INLINE void setMediaType(MPMediaType_t eType){ m_eMediaType = eType; }
	MP_INLINE void setSessionIdLeft(uint64_t nId){ m_nSessionIdLeft = nId; }
	MP_INLINE uint64_t getSessionIdLeft(){ return m_nSessionIdLeft; }
	MP_INLINE void setSessionIdRight(uint64_t nId){ m_nSessionIdRight = nId; }
	MP_INLINE uint64_t getSessionIdRight(){ return m_nSessionIdRight; }
	MP_INLINE MPObjectWrapper<MPSipSessionAV*> getCallSessionLeft(){ return m_oCallSessionLeft; }
	MP_INLINE MPObjectWrapper<MPSipSessionAV*> getCallSessionRight(){ return m_oCallSessionRight; }
	void setCallSessionRight(MPObjectWrapper<MPSipSessionAV*> oCallSession);
	MP_INLINE bool isSessionLeftActive(){ 
		switch(m_eSessionLeftState){
			case MPPeerState_Connecting: case MPPeerState_Connected: return true;
			default: return false;
		} 
	}
	MP_INLINE bool isSessionRightActive(){ 
		switch(m_eSessionRightState){
			case MPPeerState_Connecting: case MPPeerState_Connected: return true;
			default: return false;
		}
	}
	MP_INLINE bool isSessionLeftConnected(){ return (m_eSessionLeftState == MPPeerState_Connected); }
	MP_INLINE bool isSessionRightConnected(){ return (m_eSessionRightState == MPPeerState_Connected); }
	MP_INLINE void setSessionLeftState(MPPeerState_t eState){ 
		m_eSessionLeftState = eState; 
		if(m_oCallSessionLeft){
			m_oCallSessionLeft->setState(MPPeerState_GetSessionState(eState));
		}
	}
	MP_INLINE void setSessionRightState(MPPeerState_t eState){
		m_eSessionRightState = eState; 
		if(m_oCallSessionRight){
			m_oCallSessionRight->setState(MPPeerState_GetSessionState(eState));
		}
	}
	MP_INLINE MPPeerState_t getSessionLeftState(){ return m_eSessionLeftState; }
	MP_INLINE MPPeerState_t getSessionRightState(){ return m_eSessionRightState; }
	MP_INLINE void setLastSipResponseLeft(short nCode){ m_nLastSipResponseLeft = nCode; }
	MP_INLINE void setLastSipResponseRight(short nCode){ m_nLastSipResponseRight = nCode; }
	MP_INLINE short getLastSipResponseLeft(){ return m_nLastSipResponseLeft; }
	MP_INLINE short getLastSipResponseRight(){ return m_nLastSipResponseRight; }

private:
	uint64_t m_nId, m_nSessionIdLeft, m_nSessionIdRight;
	short m_nLastSipResponseLeft, m_nLastSipResponseRight;
	MPObjectWrapper<MPSipSessionAV*> m_oCallSessionLeft;
	MPObjectWrapper<MPSipSessionAV*> m_oCallSessionRight;
	MPPeerState_t m_eSessionLeftState, m_eSessionRightState;
	MPMediaType_t m_eMediaType;
};

} // namespace
#endif /* _MEDIAPROXY_PEER_H_ */
