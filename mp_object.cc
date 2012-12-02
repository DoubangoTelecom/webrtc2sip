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
#include "mp_object.h"

namespace webrtc2sip {

MPObject::MPObject()
{
	m_nRefCount = 0;
}

MPObject::MPObject(const MPObject &)
{
	m_nRefCount = 0;
}

MPObject::~MPObject()
{

}

void MPObject::operator=(const MPObject &){
}

int MPObject::takeRef() const{
	// lock
	m_nRefCount++;
	// unlock
	return m_nRefCount;
}

int MPObject::releaseRef() const{
	// lock
	if(m_nRefCount){
		m_nRefCount--;
	}
	// unlock
	return m_nRefCount;
}

} // namespace