/* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
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
#if !defined(_MEDIAPROXY_DB_MODEL_H_)
#define _MEDIAPROXY_DB_MODEL_H_

#include "mp_config.h"
#include "mp_object.h"
#include "mp_common.h"
#include "mp_db_model.h"

#include "tsk_string.h"

#include <string>

namespace webrtc2sip {

#define MPDB_ACCOUNT_INVALID_ID		-1
#define MPDB_ACCOUNT_SIP_INVALID_ID	-1

class MPDbAccountSip : public MPObject
{
public:
	MPDbAccountSip(int64_t nId, const char* pcAddress64, int64_t nAccountId);
	virtual ~MPDbAccountSip();
	MP_INLINE virtual const char* getObjectId() { return "MPDbAccountSip"; }
	MP_INLINE virtual int64_t getId() { return m_nId; }
	MP_INLINE virtual int64_t getAccountId() { return m_nAccountId; }
	MP_INLINE virtual const char* getAddress64() { return m_pAddress64; }

private:
	int64_t m_nId;
	char* m_pAddress64;
	int64_t m_nAccountId;
};

class MPDbAccountSipCaller : public MPObject
{
public:
	MPDbAccountSipCaller(int64_t nId, const char* pcDisplayName, const char* pcImpu, const char* pcImpi, const char* pcRealm, const char* pcHa1, int64_t nAccountSipId);
	virtual ~MPDbAccountSipCaller();
	MP_INLINE virtual const char* getObjectId() { return "MPDbAccountSipCaller"; }
	MP_INLINE virtual int64_t getId() { return m_nId; }
	MP_INLINE virtual const char* getDisplayName() { return m_pDisplayName; }
	MP_INLINE virtual const char* getImpu() { return m_pImpu; }
	MP_INLINE virtual const char* getImpi() { return m_pImpi; }
	MP_INLINE virtual const char* getRealm() { return m_pRealm; }
	MP_INLINE virtual const char* getHa1() { return m_pHa1; }
	MP_INLINE virtual int64_t getAccountSipId() { return m_nAccountSipId; }

private:
	int64_t m_nId;
	char* m_pDisplayName;
	char* m_pImpu;
	char* m_pImpi;
	char* m_pRealm;
	char* m_pHa1;
	int64_t m_nAccountSipId;
};

class MPDbAccount : public MPObject
{
public:
	MPDbAccount(int64_t nId, int32_t nSoftVersion, int32_t nDatabaseVersion, const char* pcName, const char* pcEmail, const char* pcPassword = NULL, const char* pcAuthToken = NULL, bool bActivated = false, const char* pcActivationCode = NULL, uint64_t epoch = 0);
	virtual ~MPDbAccount();
	MP_INLINE virtual const char* getObjectId() { return "MPDbAccount"; }
	MP_INLINE virtual int64_t getId(){ return m_nId; }
	MP_INLINE virtual int32_t getSoftVersion(){ return m_nSoftVersion; }
	MP_INLINE virtual void setSoftVersion(int32_t value){ m_nSoftVersion = value; }
	MP_INLINE virtual int32_t getDatabaseVersion(){ return m_nDatabaseVersion; }
	MP_INLINE virtual void setDatabaseVersion(int32_t value){ m_nDatabaseVersion = value; }
	MP_INLINE virtual bool isActivated(){ return m_bActivated; }
	MP_INLINE virtual void setActivated(bool value){ m_bActivated = value; }
	MP_INLINE virtual const char* getName(){ return m_pName; }
	MP_INLINE virtual void setName(const char* value){ tsk_strupdate(&m_pName, value); }
	MP_INLINE virtual const char* getEmail(){ return m_pEmail; }
	MP_INLINE virtual void setEmail(const char* value){ tsk_strupdate(&m_pEmail, value); }
	MP_INLINE virtual const char* getPassword(){ return m_pPassword; }
	MP_INLINE virtual void setPassword(const char* value){ tsk_strupdate(&m_pPassword, value); }
	MP_INLINE virtual const char* getAuthToken(){ return m_pAuthToken; }
	MP_INLINE virtual void setAuthToken(const char* value){ tsk_strupdate(&m_pAuthToken, value); }
	MP_INLINE virtual const char* getActivationCode(){ return m_pActivationCode; }
	MP_INLINE virtual void setActivationCode(const char* value){ tsk_strupdate(&m_pActivationCode, value); }
	MP_INLINE virtual uint64_t getEpoch(){ return m_nEpoch; }

	MPObjectWrapper<MPData*> serialize();
	
	static std::string buildRandomActivationCode();
	static std::string buildRandomPassword();
	static std::string buildAuthToken(const char* pcPassword, const char* pcEmail);

private:
	int64_t m_nId;
	int32_t m_nSoftVersion;
	int32_t m_nDatabaseVersion;
	char* m_pName;
	char* m_pEmail;
	char* m_pPassword;
	char* m_pAuthToken;
	bool m_bActivated;
	char* m_pActivationCode;
	uint64_t m_nEpoch;
};


}//namespace
#endif /* _MEDIAPROXY_DB_MODEL_H_ */
