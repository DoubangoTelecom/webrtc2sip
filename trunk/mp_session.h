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
#if !defined(_MEDIAPROXY_SIPSESSION_H_)
#define _MEDIAPROXY_SIPSESSION_H_

#include "mp_config.h"
#include "mp_object.h"

#include "SipSession.h"

namespace webrtc2sip {

class MPSipSession : public MPObject
{
public:
	MPSipSession(SipSession** ppSipSessionToWrap);
	virtual ~MPSipSession();
	MP_INLINE virtual const char* getObjectId() { return "MPSipSession"; }
	MP_INLINE virtual const SipSession* getWrappedSession(){ return m_pWrappedSession; }
	MP_INLINE virtual void setState(MPSessionState_t eState) { m_eState = eState; }
	MP_INLINE virtual MPSessionState_t getState(){ return m_eState; }
private:
	SipSession* m_pWrappedSession;
	MPSessionState_t m_eState;
};

} // namespace

#endif /* _MEDIAPROXY_SIPSESSION_H_ */
