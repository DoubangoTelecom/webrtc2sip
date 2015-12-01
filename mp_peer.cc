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
#include "mp_peer.h"

#include <assert.h>

namespace webrtc2sip {

uint64_t g_nId = 0;

MPPeer::MPPeer(MPObjectWrapper<MPSipSessionAV*> oCallSessionLeft)
: m_nSessionIdLeft(0)
, m_nSessionIdRight(0)
, m_eSessionLeftState(MPPeerState_None)
, m_eSessionRightState(MPPeerState_None)
, m_nLastSipResponseLeft(0)
, m_nLastSipResponseRight(0)
, m_eMediaType(MPMediaType_None)
{
	assert(*oCallSessionLeft);

	m_nId = ++g_nId;
	m_oCallSessionLeft = oCallSessionLeft;
	m_nSessionIdLeft = m_oCallSessionLeft->getWrappedSession()->getId();
}

MPPeer::~MPPeer()
{
	TSK_DEBUG_INFO("MPPeer object destroyed");
}

void MPPeer::setCallSessionRight(MPObjectWrapper<MPSipSessionAV*> oCallSession)
{
	assert(*oCallSession);
	
	m_oCallSessionRight = oCallSession;
	m_nSessionIdRight = m_oCallSessionRight->getWrappedSession()->getId();
}

} // namespace