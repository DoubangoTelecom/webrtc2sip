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
#if !defined(_MEDIAPROXY_ENGINE_H_)
#define _MEDIAPROXY_ENGINE_H_

#include "mp_config.h"
#include "mp_object.h"
#include "mp_mutex.h"
#include "mp_wrap.h"
#include "mp_peer.h"

#include <map>

namespace webrtc2sip {

//
//	MPEngine
//
class MPEngine : public MPObject
{
	friend class MPSipCallback;
protected:
	MPEngine(const char* pRealmUri, const char* pPrivateIdentity, const char* pPublicIdentity);
public:
	virtual ~MPEngine();
	virtual MP_INLINE const char* getObjectId() { return "MPEngine"; }
	virtual MP_INLINE bool isValid(){ return  m_bValid; }
	virtual MP_INLINE bool isStarted(){ return m_bStarted; }
	virtual bool setDebugLevel(const char* pcLevel);
	virtual bool addTransport(const char* pTransport, uint16_t pLocalPort, const char* pLocalIP = tsk_null);
	virtual bool set100relEnabled(bool bEnabled);
	virtual bool setMediaCoderEnabled(bool bEnabled);
	virtual bool setVideoJbEnabled(bool bEnabled);
	virtual bool setRtpBuffSize(int32_t nSize);
	virtual bool setAvpfTail(int32_t nMin, int32_t nMax);
	virtual bool setSSLCertificate(const char* pcPrivateKey, const char* pcPublicKey, const char* pcCA);
	virtual bool setCodecs(int64_t nCodecs);
	virtual bool addDNSServer(const char* pcDNSServer);
	virtual bool start();
	virtual bool stop();

	static MPObjectWrapper<MPEngine*> New();


protected:
	virtual MP_INLINE void setStarted(bool bStarted){ m_bStarted = bStarted; }
	virtual MPObjectWrapper<MPPeer*> getPeerById(uint64_t nId);
	virtual MPObjectWrapper<MPPeer*> getPeerBySessionId(uint64_t nId, bool bSessionLeft);
	virtual void insertPeer(MPObjectWrapper<MPPeer*> oPeer);
	virtual void removePeer(uint64_t nId);

private:
	MPObjectWrapper<MPMutex*> m_oMutex;
	MPObjectWrapper<MPSipCallback*> m_oCallback;
	MPObjectWrapper<MPSipStack*> m_oStack;

	std::map<uint64_t, MPObjectWrapper<MPPeer*> > m_Peers;
	MPObjectWrapper<MPMutex*> m_oMutexPeers;

	bool m_bStarted;
	bool m_bValid;

	static bool g_bInitialized;
};


} // namespace
#endif /* _MEDIAPROXY_ENGINE_H_ */
