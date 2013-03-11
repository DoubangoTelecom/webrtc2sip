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
#if !defined(_MEDIAPROXY_DB_SQLITE_H_)
#define _MEDIAPROXY_DB_SQLITE_H_

#include "mp_config.h"
#include "db/mp_db.h"

struct sqlite3;

namespace webrtc2sip {

class MPDbSQLite : public MPDb
{
public:
	MPDbSQLite(const char* pcConnectionInfo);
	virtual ~MPDbSQLite();
	MP_INLINE virtual const char* getObjectId() { return "MPDbSQLite"; }

	// override(MPDb)
	virtual bool open();
	virtual bool isOpened(){ return m_bOpened; }
	virtual bool killZombieAccounts(uint64_t nMaxLifeTimeExpectancyInMillis);
	virtual bool addAccount(MPObjectWrapper<MPDbAccount*> oAccount);
	virtual bool addAccountSip(MPObjectWrapper<MPDbAccountSip*> oAccountSip);
	virtual bool addAccountSipCaller(MPObjectWrapper<MPDbAccountSipCaller*> oAccountSipCaller);
	virtual bool updateAccount(MPObjectWrapper<MPDbAccount*> oAccount);
	virtual MPObjectWrapper<MPDbAccount*> selectAccountByEmail(const char* pcEmail);
	virtual MPObjectWrapper<MPDbAccountSip*> selectAccountSipByAddress(const char* pcAddress);
	virtual MPObjectWrapper<MPDbAccountSip*> selectAccountSipByAccountId(int64_t nAccountId);
	virtual MPObjectWrapper<MPDbAccountSip*> selectAccountSipByAccountIdAndAddress(int64_t nAccountId, const char* pcAddress);
	virtual MPObjectWrapper<MPDbAccountSipCaller*> selectAccountSipCallerById(int64_t nId);
	virtual MPObjectWrapper<MPDbAccountSipCaller*> selectAccountSipCallerByAccountSipId(int64_t nAccountSipId);
	virtual bool deleteAccountSipById(uint64_t id);
	virtual bool deleteAccountSipCallerById(uint64_t id);
	virtual bool close();

private:
	virtual MPObjectWrapper<MPDbAccountSipCaller*> selectAccountSipCaller(int64_t nId, bool bByAccountSipId);
	static int dbStep(void* pCompiledStatement);

private:
	struct sqlite3* m_pEngine;
	bool m_bOpened;
};

} // namespace
#endif /* _MEDIAPROXY_DB_SQLITE_H_ */
