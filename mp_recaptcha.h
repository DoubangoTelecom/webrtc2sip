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
#if !defined(_MEDIAPROXY_RECAPTCHA_H_)
#define _MEDIAPROXY_RECAPTCHA_H_

#include "mp_config.h"
#include "mp_common.h"
#include "mp_net_transport.h"

#include <map>

struct thttp_url_s;

namespace webrtc2sip {

class MPRecaptchaTransport;

//
//	MPRecaptcha
//
class MPRecaptcha : public MPObject
{
public:
	MPRecaptcha(MPNetFd nFd, const char* pcAccountName, const char* pcAccountEmail, const char* pcResponse, const char* pcRemoteIP = NULL);
	virtual ~MPRecaptcha();
	virtual MP_INLINE const char* getObjectId() { return "MPRecaptcha"; }
	virtual MP_INLINE const char* getResponse() { return m_pResponse; }
	virtual MP_INLINE const char* getRemoteIP() { return m_pRemoteIP; }
	virtual MP_INLINE const char* getAccountName() { return m_pAccountName; }
	virtual MP_INLINE const char* getAccountEmail() { return m_pAccountEmail; }
	virtual MP_INLINE MPNetFd getFd() { return m_nFd; }
	virtual MP_INLINE bool isConnected() { return m_bConnected; }
	virtual MP_INLINE void setConnected(bool bConnected) { m_bConnected = bConnected; }
	virtual void setRemoteIP(const char* pcRemoteIP);

private:
	char* m_pResponse;
	char* m_pRemoteIP;
	char* m_pAccountName;
	char* m_pAccountEmail;
	MPNetFd m_nFd;
	bool m_bConnected;
};

//
//	MPRecaptchaTransportCallback
//
class MPRecaptchaTransportCallback : public MPNetTransportCallback
{
public:
	MPRecaptchaTransportCallback(const MPRecaptchaTransport* pcTransport);
	virtual ~MPRecaptchaTransportCallback();
	virtual MP_INLINE const char* getObjectId() { return "MPRecaptchaTransportCallback"; }
	virtual bool onData(MPObjectWrapper<MPNetPeer*> oPeer, size_t &nConsumedBytes);
	virtual bool onConnectionStateChanged(MPObjectWrapper<MPNetPeer*> oPeer);
private:
	const MPRecaptchaTransport* m_pcTransport;
};

//
//	MPRecaptchaValidationCallback
//
class MPRecaptchaValidationCallback: public MPObject
{
public:
	MPRecaptchaValidationCallback() {}
	virtual ~MPRecaptchaValidationCallback() {}
	virtual MP_INLINE const char* getObjectId() { return "MPRecaptchaValidationCallback"; }
	virtual bool onValidationEvent(bool bSucess, MPObjectWrapper<MPRecaptcha*> oRecaptcha) = 0;
};

//
//	MPRecaptchaTransport
//
class MPRecaptchaTransport : public MPNetTransport
{
	friend class MPRecaptchaTransportCallback;
public:
	MPRecaptchaTransport(const char* pcVerifySiteUrl, const char* pcSecret);
	virtual ~MPRecaptchaTransport();
	virtual MP_INLINE const char* getObjectId() { return "MPRecaptchaTransport"; }
	virtual MP_INLINE void setValidationCallback(MPObjectWrapper<MPRecaptchaValidationCallback*> oCallback) { m_oValidationCallback = oCallback; };
	virtual bool validate(MPNetFd nLocalFd, const char* pcAccountName, const char* pcAccountEmail, const char* pcResponse);

private:
	void insertRecaptcha(MPObjectWrapper<MPRecaptcha*> oRecaptcha);
	void removeRecaptcha(MPNetFd nFd);
	MPObjectWrapper<MPRecaptcha*> getRecaptchaFd(MPNetFd nFd);
	bool sendRecaptcha(MPObjectWrapper<MPRecaptcha*> oRecaptcha);

private:
	MPObjectWrapper<MPRecaptchaTransportCallback*> m_oCallback;
	MPObjectWrapper<MPRecaptchaValidationCallback*> m_oValidationCallback;
	std::map<MPNetFd, MPObjectWrapper<MPRecaptcha*> > m_Recaptchas;
	char* m_pVerifySiteUrl;
	char* m_pSecret;
	struct thttp_url_s* m_pWrappedHttpUrl;
	tsk_mutex_handle_t *m_pWrappedRecaptchasMutex;
};

}// namespace
#endif /* _MEDIAPROXY_RECAPTCHA_H_ */
