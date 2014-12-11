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
#if !defined(_MEDIAPROXY_NET_TRANSPORT_H_)
#define _MEDIAPROXY_NET_TRANSPORT_H_

#include "mp_config.h"
#include "mp_object.h"
#include "mp_common.h"

#include "tnet_transport.h"
#include "tsk_mutex.h"
#include "tsk_buffer.h"

#include <map>

namespace webrtc2sip {

class MPNetTransport;
class MPNetTransportCallback;

//
//	MPNetPeer
//
class MPNetPeer : public MPObject
{
	friend class MPNetTransport;
	friend class MPNetTransportCallback;
public:
	MPNetPeer(MPNetFd nFd, bool bConnected = false, const void* pcData = NULL, size_t nDataSize = 0)
	{
		m_bConnected = bConnected;
		m_nFd = nFd;
		m_pWrappedBuffer = tsk_buffer_create(pcData, nDataSize);
	}
	virtual ~MPNetPeer()
	{
		TSK_OBJECT_SAFE_FREE(m_pWrappedBuffer);
	}
	virtual MP_INLINE MPNetFd getFd(){ return  m_nFd; }
	virtual MP_INLINE bool isConnected(){ return  m_bConnected; }
	virtual MP_INLINE const void* getDataPtr() { return m_pWrappedBuffer ? m_pWrappedBuffer->data : NULL; }
	virtual MP_INLINE size_t getDataSize() { return m_pWrappedBuffer ? m_pWrappedBuffer->size : 0; }
	virtual bool sendData(const void* pcDataPtr, size_t nDataSize);
	virtual MP_INLINE bool isStream() = 0;

protected:
	virtual MP_INLINE void setConnected(bool bConnected) { m_bConnected = bConnected; }

protected:
	bool m_bConnected;
	MPNetFd m_nFd;
	tsk_buffer_t* m_pWrappedBuffer;
};


//
//	MPNetPeerDgram
//
class MPNetPeerDgram : public MPNetPeer
{
public:
	MPNetPeerDgram(MPNetFd nFd, const void* pcData = NULL, size_t nDataSize = 0)
		:MPNetPeer(nFd, false, pcData, nDataSize)
	{
	}
	virtual ~MPNetPeerDgram()
	{
	}
	virtual MP_INLINE const char* getObjectId() { return "MPNetPeerDgram"; }
	virtual MP_INLINE bool isStream(){ return false; }
};

//
//	MPNetPeerStream
//
class MPNetPeerStream : public MPNetPeer
{
public:
	MPNetPeerStream(MPNetFd nFd, bool bConnected = false, const void* pcData = NULL, size_t nDataSize = 0)
		:MPNetPeer(nFd, bConnected, pcData, nDataSize)
	{
	}
	virtual ~MPNetPeerStream()
	{
	}
	virtual MP_INLINE const char* getObjectId() { return "MPNetPeerStream"; }
	virtual MP_INLINE bool isStream(){ return true; }
	virtual MP_INLINE bool appenData(const void* pcData, size_t nDataSize){ return m_pWrappedBuffer ? tsk_buffer_append(m_pWrappedBuffer, pcData, nDataSize) == 0 : false; }
	virtual MP_INLINE bool remoteData(size_t nPosition, size_t nSize){ return m_pWrappedBuffer ? tsk_buffer_remove(m_pWrappedBuffer, nPosition, nSize) == 0 : false; }
	virtual MP_INLINE bool cleanupData(){ return m_pWrappedBuffer ? tsk_buffer_cleanup(m_pWrappedBuffer) == 0 : false; }
};

//
//	MPNetTransport
//
class MPNetTransportCallback : public MPObject
{
public:
	MPNetTransportCallback()
	{
	}
	virtual ~MPNetTransportCallback()
	{
	}
	virtual bool onData(MPObjectWrapper<MPNetPeer*> oPeer, size_t &nConsumedBytes) = 0;
	virtual bool onConnectionStateChanged(MPObjectWrapper<MPNetPeer*> oPeer) = 0;
};

//
//	MPNetTransport
//
class MPNetTransport : public MPObject
{
protected:
	MPNetTransport(MPNetTransporType_t eType, const char* pcLocalIP, unsigned short nLocalPort);
public:
	virtual ~MPNetTransport();
	virtual MP_INLINE MPNetTransporType_t getType() { return m_eType; }
	virtual MP_INLINE bool isStarted(){ return m_bStarted; }
	virtual MP_INLINE bool isValid(){ return m_bValid; }
	virtual MP_INLINE void setCallback(MPObjectWrapper<MPNetTransportCallback*> oCallback) { m_oCallback = oCallback; }

	virtual bool setSSLCertificates(const char* pcPrivateKey, const char* pcPublicKey, const char* pcCA, bool bVerify = false);
	virtual bool start();
	virtual MPNetFd connectTo(const char* pcHost, unsigned short nPort);
	virtual bool isConnected(MPNetFd nFd);
	virtual bool sendData(MPNetFd nFdFrom, const void* pcDataPtr, size_t nDataSize);
	virtual bool stop();

private:
	MPObjectWrapper<MPNetPeer*> getPeerByFd(MPNetFd nFd);
	void insertPeer(MPObjectWrapper<MPNetPeer*> oPeer);
	void removePeer(MPNetFd nFd);
	static int MPNetTransportCb_Stream(const tnet_transport_event_t* e);

protected:
	tnet_transport_handle_t* m_pWrappedTransport;
	MPNetTransporType_t m_eType;
	bool m_bValid, m_bStarted;
	std::map<MPNetFd, MPObjectWrapper<MPNetPeer*> > m_Peers;
	MPObjectWrapper<MPNetTransportCallback*> m_oCallback;
	tsk_mutex_handle_t *m_pWrappedPeersMutex;
};




}//namespace
#endif /* _MEDIAPROXY_NET_TRANSPORT_H_ */
