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
#if !defined(_MEDIAPROXY_PROXYPLUGIN_H_)
#define _MEDIAPROXY_PROXYPLUGIN_H_

#include "mp_config.h"
#include "mp_object.h"
#include "mp_mutex.h"

#include "ProxyPluginMgr.h"

namespace webrtc2sip {

class MPProxyPlugin : public MPObject
{
protected:
	MPProxyPlugin(MPMediaType_t eMediaType, uint64_t nId, const ProxyPlugin* pcProxyPlugin);

public:
	virtual ~MPProxyPlugin();
	MP_INLINE virtual const char* getObjectId() { return "MPProxyPlugin"; }
	MP_INLINE bool operator ==(const MPProxyPlugin& other) const{
		return m_nId == other.m_nId;
	}
	MP_INLINE virtual uint64_t getId() { return m_nId; }
	MP_INLINE virtual MPMediaType_t getMediaType() { return m_eMediaType; }
	MP_INLINE virtual void invalidate(){ m_bValid = false; }
	MP_INLINE virtual bool isValid() { return m_bValid; }
	MP_INLINE virtual bool isPrepared() { return m_bPrepared; }
	MP_INLINE virtual bool isPaused() { return m_bPaused; }
	MP_INLINE virtual bool isStarted() { return m_bStarted; }

protected:
	MP_INLINE int prepare() { m_bPrepared = true; return 0; }
	MP_INLINE int start() { m_bStarted = true; return 0; }
	MP_INLINE int pause() { m_bPaused = true; return 0; }
	MP_INLINE int stop() { m_bStarted = false; return 0; }

protected:
	bool m_bValid;
	bool m_bPrepared;
	bool m_bStarted;
	bool m_bPaused;
	uint64_t m_nId;
	MPMediaType_t m_eMediaType;
	const ProxyPlugin* m_pcProxyPlugin;
	MPObjectWrapper<MPMutex*> m_oMutex;	
};

} // namespace
#endif /* _MEDIAPROXY_PROXYPLUGIN_H_ */
