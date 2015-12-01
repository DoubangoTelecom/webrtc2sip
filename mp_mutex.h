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
#if !defined(_MEDIAPROXY_MUTEX_H_)
#define _MEDIAPROXY_MUTEX_H_

#include "mp_config.h"
#include "mp_object.h"
#include "tsk_mutex.h"

namespace webrtc2sip {

class MPMutex : public MPObject
{
protected:
	MPMutex(bool bRecursive = true);
public:
	virtual ~MPMutex();
	virtual MP_INLINE const char* getObjectId() { return "MPMutex"; }
	bool lock();
	bool unlock();
	static MPObjectWrapper<MPMutex*> New(bool bRecursive = true);

private:
	tsk_mutex_handle_t* m_phMPMutex;
};

}// namespace
#endif /* _MEDIAPROXY_MUTEX_H_ */
