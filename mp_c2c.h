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
#if !defined(_MEDIAPROXY_CLICK2CALL_H_)
#define _MEDIAPROXY_CLICK2CALL_H_

#include "mp_config.h"
#include "mp_net_transport.h"
#include "tsk_debug.h"
#include "db/mp_db.h"
#include "mp_mail.h"
#include "mp_recaptcha.h"

namespace webrtc2sip {

class MPC2CTransport;

//
//	MPC2CResult
// 
class MPC2CResult : public MPObject
{
public:
	MPC2CResult(unsigned short nCode, const char* pcPhrase, const void* pcDataPtr = NULL, size_t nDataSize = 0);
	virtual ~MPC2CResult();
	virtual MP_INLINE const char* getObjectId() { return "MPC2CResult"; }

	virtual MP_INLINE unsigned short getCode() { return m_nCode; }
	virtual MP_INLINE const char* getPhrase() { return m_pPhrase; }
	virtual MP_INLINE const void* getDataPtr() { return m_pDataPtr; }
	virtual MP_INLINE size_t getDataSize() { return m_nDataSize; }

private:
	unsigned short m_nCode;
	char* m_pPhrase;
	void* m_pDataPtr;
	size_t m_nDataSize;
};

//
//	MPC2CTransportCallback
//
class MPC2CTransportCallback : public MPNetTransportCallback
{
public:
	MPC2CTransportCallback(const MPC2CTransport* pcTransport);
	virtual ~MPC2CTransportCallback();
	virtual MP_INLINE const char* getObjectId() { return "MPC2CTransportCallback"; }
	virtual bool onData(MPObjectWrapper<MPNetPeer*> oPeer, size_t &nConsumedBytes);
	virtual bool onConnectionStateChanged(MPObjectWrapper<MPNetPeer*> oPeer);
private:
	const MPC2CTransport* m_pcTransport;
};

//
// MPC2CRecaptchaValidationCallback
//
class MPC2CRecaptchaValidationCallback: public MPRecaptchaValidationCallback
{
public:
	MPC2CRecaptchaValidationCallback(const MPC2CTransport* pcTransport);
	virtual ~MPC2CRecaptchaValidationCallback();
	virtual MP_INLINE const char* getObjectId() { return "MPC2CRecaptchaValidationCallback"; }
	virtual bool onValidationEvent(bool bSucess, MPObjectWrapper<MPRecaptcha*> oRecaptcha);
private:
	const MPC2CTransport* m_pcTransport;
};

//
//	MPC2CTransport
//
class MPC2CTransport : public MPNetTransport
{
	friend class MPC2CTransportCallback;
	friend class MPC2CRecaptchaValidationCallback;
public:
	MPC2CTransport(bool isSecure, const char* pcLocalIP, unsigned short nLocalPort);
	virtual ~MPC2CTransport();
	virtual MP_INLINE const char* getObjectId() { return "MPC2CTransport"; }
	virtual void setDb(MPObjectWrapper<MPDb*> oDb);
	virtual void setMailTransport(MPObjectWrapper<MPMailTransport*> oMailTransport);
	virtual void setRecaptchaTransport(MPObjectWrapper<MPRecaptchaTransport*> oRecaptchaTransport);

private:
	MPObjectWrapper<MPData*> serializeAccount(const char* pcActionId, MPObjectWrapper<MPDbAccount*> oAccount)const;
	MPObjectWrapper<MPC2CResult*> handleJsonContent(MPNetFd nFd, const void* pcDataPtr, size_t nDataSize)const;
	MPObjectWrapper<MPC2CResult*> addAccountAndSendActivationMail(const char* pcEmail, const char* pcName)const;
	bool sendActivationMail(const char* pcEmail, const char* pcName, const char* pcActivationCode, const char* pcPassword)const;
	bool sendResult(MPObjectWrapper<MPNetPeer*> oPeer, MPObjectWrapper<MPC2CResult*> oResult)const;

private:
	MPObjectWrapper<MPC2CTransportCallback*> m_oCallback;
	MPObjectWrapper<MPC2CRecaptchaValidationCallback*> m_oRecaptchaValidationCallback;
	MPObjectWrapper<MPDb*> m_oDb;
	MPObjectWrapper<MPMailTransport*> m_oMailTransport;
	MPObjectWrapper<MPRecaptchaTransport*> m_oRecaptchaTransport;
	static uint64_t s_nAccountsCount;
};

}// namespace
#endif /* _MEDIAPROXY_CLICK2CALL_H_ */
