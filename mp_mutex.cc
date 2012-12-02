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
#include "mp_mutex.h"

namespace webrtc2sip {

MPMutex::MPMutex(bool bRecursive /*= true*/)
{
	m_phMPMutex = tsk_mutex_create_2(bRecursive ? tsk_true : tsk_false);
}

MPMutex::~MPMutex()
{
	if(m_phMPMutex){
		tsk_mutex_destroy(&m_phMPMutex);
	}
}

bool MPMutex::lock()
{
	return (tsk_mutex_lock(m_phMPMutex) == 0);
}

bool MPMutex::unlock()
{
	return (tsk_mutex_unlock(m_phMPMutex) == 0);
}

MPObjectWrapper<MPMutex*> MPMutex::New(bool bRecursive /*= true*/)
{
	MPObjectWrapper<MPMutex*> oMPMutex = new MPMutex(bRecursive);
	if(!oMPMutex->m_phMPMutex){
		TSK_DEBUG_ERROR("Not wrapping valid mutex handle");
		MPObjectSafeRelease(oMPMutex);
	}
	return oMPMutex;
}

}// namespace