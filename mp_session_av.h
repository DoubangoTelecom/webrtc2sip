/* Copyright (C) 2012-2015 Doubango Telecom <http://www.doubango.org>
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
#if !defined(_MEDIAPROXY_SIPSESSIONAV_H_)
#define _MEDIAPROXY_SIPSESSIONAV_H_

#include "mp_config.h"
#include "mp_object.h"
#include "mp_session.h"

namespace webrtc2sip {


class MPSipSessionAVRtcpCallback;

//
//	MPSipSessionAV
//
class MPSipSessionAV : public MPSipSession
{
public:
	MPSipSessionAV(CallSession** ppCallSessionToWrap, MPMediaType_t eMediaType);
	virtual ~MPSipSessionAV();
	MP_INLINE virtual const char* getObjectId() { return "MPSipSessionAV"; }


public:
	MP_INLINE virtual const CallSession* getWrappedCallSession(){ return dynamic_cast<const CallSession*>(getWrappedSession()); }
	MP_INLINE virtual const InviteSession* getWrappedInviteSession(){ return dynamic_cast<const InviteSession*>(getWrappedSession()); }
	MP_INLINE virtual void setSessionOpposite(MPObjectWrapper<MPSipSessionAV*> oSessionOpposite){ m_oSessionOpposite = oSessionOpposite; }
	virtual void setState(MPSessionState_t eState); // @Override
	virtual int onRtcpEventCallback(const RtcpCallbackData* pcEvent)const;

private:
	MPObjectWrapper<MPSipSessionAVRtcpCallback*> m_oRtcpCallback;
	MPObjectWrapper<MPSipSessionAV*> m_oSessionOpposite;
	MPMediaType_t m_eMediaType;
};


//
//	MPSipSessionAVRtcpCallback
//
class MPSipSessionAVRtcpCallback : public RtcpCallback, public MPObject {
public:
	MPSipSessionAVRtcpCallback(const MPSipSessionAV* pcSessionAV)
	  : RtcpCallback(), m_pcSessionAV(pcSessionAV){
	}
	virtual ~MPSipSessionAVRtcpCallback(){
	}
	MP_INLINE virtual const char* getObjectId() { return "MPSipSessionAVRtcpCallback"; }
public: /* Overrides */
	virtual int onevent(const RtcpCallbackData* e){ 
		return m_pcSessionAV->onRtcpEventCallback(e); 
	}
private:
	const MPSipSessionAV* m_pcSessionAV;
};

} // namespace

#endif /* _MEDIAPROXY_SIPSESSIONAV_H_ */
