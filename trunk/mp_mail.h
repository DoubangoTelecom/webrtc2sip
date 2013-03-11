/* Copyright (C) 2012-2013 Doubango Telecom <http://www.doubango.org>
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
#if !defined(_MEDIAPROXY_MAIL_H_)
#define _MEDIAPROXY_MAIL_H_

#include "mp_config.h"
#include "mp_net_transport.h"

#include <list>

namespace webrtc2sip {

class MPMailTransport;


//
//	MPMail
//
class MPMail : public MPObject
{
public:
	MPMail(const char* pcSrcMailAddr, const char* pcDstMailAddr, const char* pcSubject, const void* pcDataPtr, size_t nDataSize);
	virtual ~MPMail();
	virtual MP_INLINE const char* getObjectId() { return "MPMail"; }
	virtual MP_INLINE const char* getSrcMailAddr() { return m_pSrcMailAddr; }
	virtual MP_INLINE const char* getDstMailAddr() { return m_pDstMailAddr; }
	virtual MP_INLINE const char* getSubject() { return m_pSubject; }
	virtual MP_INLINE const void* getDataPtr() { return m_pDataPtr; }
	virtual MP_INLINE size_t getDataSize() { return m_nDataSize; }

private:
	char* m_pSrcMailAddr;
	char* m_pDstMailAddr;
	char* m_pSubject;
	void* m_pDataPtr;
	size_t m_nDataSize;
};

//
//	MPMailTransportCallback
//
class MPMailTransportCallback : public MPNetTransportCallback
{
public:
	MPMailTransportCallback(const MPMailTransport* pcTransport);
	virtual ~MPMailTransportCallback();
	virtual MP_INLINE const char* getObjectId() { return "MPMailTransportCallback"; }
	virtual bool onData(MPObjectWrapper<MPNetPeer*> oPeer, size_t &nConsumedBytes);
	virtual bool onConnectionStateChanged(MPObjectWrapper<MPNetPeer*> oPeer);
private:
	const MPMailTransport* m_pcTransport;
};

//
//	MPMailTransport
//
class MPMailTransport : public MPNetTransport
{
	friend class MPMailTransportCallback;
public:
	MPMailTransport(bool isSecure, const char* pcLocalIP, unsigned short nLocalPort, const char* pcSmtpHost, unsigned short nSmtpPort, const char* pcEmail, const char* pcAuthName, const char* pcAuthPwd);
	virtual ~MPMailTransport();
	virtual MP_INLINE const char* getObjectId() { return "MPMailTransport"; }
	virtual bool sendMail(const char* pcDstMailAddr, const char* pcSubject, const void* pcDataPr, size_t nDataSize, const char* pcSrcMailAddr = NULL);

private:
	bool sendMail(MPObjectWrapper<MPMail*> oMail);
	bool sendPendingMails();

private:
	MPObjectWrapper<MPMailTransportCallback*> m_oCallback;
	bool m_bConnectToPending;
	char* m_pSmtpHost;
	unsigned short m_nSmtpPort;
	char* m_pEmail;
	char* m_pAuthName64;
	char* m_pAuthPwd64; 
	MPNetFd m_nConnectedFd;
	std::list<MPObjectWrapper<MPMail*> > m_oPendingMails;
	uint8_t* m_pTempBuffPtr;
	size_t m_nTempBuffSize;
	bool m_bNeedToAuthenticate;
};

}// namespace
#endif /* _MEDIAPROXY_MAIL_H_ */

