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
#if !defined(_MEDIAPROXY_DB_H_)
#define _MEDIAPROXY_DB_H_

#include "mp_config.h"
#include "mp_object.h"
#include "mp_common.h"
#include "mp_db_model.h"

namespace webrtc2sip {

class MPDb : public MPObject
{
public:
	MPDb(MPDbType_t eType, const char* pcConnectionInfo);
	virtual ~MPDb();
	MP_INLINE virtual const char* getObjectId() { return "MPDb"; }

	MP_INLINE virtual MPDbType_t getType(){ return m_eType; }
	MP_INLINE virtual const char* getConnectionInfo(){ return m_pConnectionInfo; }

	virtual bool open() = 0;
	virtual bool isOpened() = 0;
	virtual bool killZombieAccounts(uint64_t nMaxLifeTimeExpectancyInMillis) = 0;
	virtual bool addAccount(MPObjectWrapper<MPDbAccount*> oAccount) = 0;
	virtual bool addAccountSip(MPObjectWrapper<MPDbAccountSip*> oAccountSip) = 0;
	virtual bool addAccountSipCaller(MPObjectWrapper<MPDbAccountSipCaller*> oAccountSipCaller) = 0;
	virtual bool updateAccount(MPObjectWrapper<MPDbAccount*> oAccount) = 0;
	virtual MPObjectWrapper<MPDbAccount*> selectAccountByEmail(const char* pcEmail) = 0;
	virtual MPObjectWrapper<MPDbAccountSip*> selectAccountSipByAddress(const char* pcAddress) = 0;
	virtual MPObjectWrapper<MPDbAccountSip*> selectAccountSipByAccountId(int64_t nAccountId) = 0;
	virtual MPObjectWrapper<MPDbAccountSip*> selectAccountSipByAccountIdAndAddress(int64_t nAccountId, const char* pcAddress) = 0;
	virtual MPObjectWrapper<MPDbAccountSipCaller*> selectAccountSipCallerById(int64_t nId) = 0;
	virtual MPObjectWrapper<MPDbAccountSipCaller*> selectAccountSipCallerByAccountSipId(int64_t nAccountSipId) = 0;
	virtual bool deleteAccountSipById(uint64_t id) = 0;
	virtual bool deleteAccountSipCallerById(uint64_t id) = 0;
	virtual bool close() = 0;

	static const char* sSQL;
	static const char* sSQL_DeleteZombies;
	static const char* sSQL_InsertAccount;
	static const char* sSQL_UpdateAccountById;
	static const char* sSQL_SelectAccountByEmail;
	static const char* sSQL_DeleteAccountSip;
	static const char* sSQL_InsertAccountSip;
	static const char* sSQL_SelectAccountSipByAddress;
	static const char* sSQL_SelectAccountSipByAccountId;
	static const char* sSQL_SelectAccountSipByAccountIdAndAddress;
	static const char* sSQL_DeleteAccountSipCaller;
	static const char* sSQL_InsertAccountSipCaller;
	static const char* sSQL_SelectAccountSipCallerById;
	static const char* sSQL_SelectAccountSipCallerByAccountSipId;

	static MPObjectWrapper<MPDb*> New(MPDbType_t eType, const char* pcConnectionInfo);

protected:
	MPDbType_t m_eType;
	char* m_pConnectionInfo;
};

class MP : public MPObject
{
public:
};

} // namespace
#endif /* _MEDIAPROXY_DB_H_ */
