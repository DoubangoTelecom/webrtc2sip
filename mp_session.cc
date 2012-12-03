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
#include "mp_session.h"

#include <assert.h>

namespace webrtc2sip {

MPSipSession::MPSipSession(SipSession** ppSipSessionToWrap)
: m_eState(MPSessionState_None)
{
	assert(ppSipSessionToWrap && *ppSipSessionToWrap);

	m_pWrappedSession = *ppSipSessionToWrap;
	*ppSipSessionToWrap = NULL;
}

MPSipSession::~MPSipSession()
{
	if(m_pWrappedSession){
		delete m_pWrappedSession, m_pWrappedSession = NULL;
	}

	TSK_DEBUG_INFO("MPSipSession object destroyed");
}


} // namespace
