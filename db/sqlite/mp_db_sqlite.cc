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
#include "mp_db_sqlite.h"

#include "tsk_time.h"
#include "tsk_thread.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include "sqlite3.h"

namespace webrtc2sip {

MPDbSQLite::MPDbSQLite(const char* pcConnectionInfo)
: MPDb(MPDbType_SQLite, pcConnectionInfo)
, m_pEngine(NULL)
, m_bOpened(false)
{
}

MPDbSQLite::~MPDbSQLite()
{
	close();
}
// MPDb::open
bool MPDbSQLite::open()
{
	close();
	
	int ret = sqlite3_open(m_pConnectionInfo, &m_pEngine);
	m_bOpened = (ret == SQLITE_OK);
	if(!m_bOpened || !m_pEngine)
	{
		TSK_DEBUG_ERROR("Failed to open SQLite database with error code = %d and connectionInfo=%s", ret, m_pConnectionInfo);
		return false;
	}

	TSK_DEBUG_INFO("sqlite3_threadsafe = %d", sqlite3_threadsafe());
	
	char* err = NULL;

	// http://www.sqlite.org/draft/wal.html
	ret = sqlite3_exec(m_pEngine, "PRAGMA journal_mode = WAL;", NULL, NULL, &err);
	ret = sqlite3_exec(m_pEngine, "PRAGMA page_size = 4096;", NULL, NULL, &err);
	ret = sqlite3_exec(m_pEngine, "PRAGMA synchronous = FULL;", NULL, NULL, &err);
	if(!(m_bOpened = (ret == SQLITE_OK)))
	{
		TSK_DEBUG_ERROR("Failed to set journal_mode value to WAL [%s]", err);
		return false;
	}
	// set database version
	ret = sqlite3_exec(m_pEngine, "PRAGMA user_version = 0;", NULL, NULL, &err);
	if(!(m_bOpened = (ret == SQLITE_OK)))
	{
		TSK_DEBUG_ERROR("Failed to set SQLite database version [%s]", err);
		return false;
	}

	// create tables
	ret = sqlite3_exec(m_pEngine, MPDb::sSQL, NULL, NULL, &err);
	if(!(m_bOpened = (ret == SQLITE_OK)))
	{
		TSK_DEBUG_ERROR("Failed to create tables [%s]", err);
		return false;
	}

	TSK_DEBUG_INFO("Database opened = %s", m_bOpened ? "TRUE" : "FALSE");

	return m_bOpened;
}

// MPDb::updateAccount
bool MPDbSQLite::updateAccount(MPObjectWrapper<MPDbAccount*> oAccount)
{
	assert(*oAccount);
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	ret = dbTransacExec("BEGIN IMMEDIATE TRANSACTION;");

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_UpdateAccountById, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_UpdateAccountById, ret);
		goto bail;
	}

	ret += sqlite3_bind_int(pCompiledStatement, 1, oAccount->getSoftVersion());
	ret += sqlite3_bind_int(pCompiledStatement, 2, oAccount->getDatabaseVersion());
	ret += sqlite3_bind_text(pCompiledStatement, 3, oAccount->getName(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 4, oAccount->getEmail(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 5, oAccount->getPassword(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 6, oAccount->getAuthToken(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_int(pCompiledStatement, 7, oAccount->isActivated() ? 1 : 0);
	ret += sqlite3_bind_text(pCompiledStatement, 8, oAccount->getActivationCode(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_int64(pCompiledStatement, 9, oAccount->getEpoch());
	// WHERE Id = ?
	ret += sqlite3_bind_int64(pCompiledStatement, 10, oAccount->getId());

	if(ret == SQLITE_OK)
	{
		ret = MPDbSQLite::dbStep(pCompiledStatement);
	}
	sqlite3_finalize(pCompiledStatement);

bail:
	dbTransacExec("COMMIT TRANSACTION;");
	return (ret == SQLITE_DONE);
}

// MPDb::killZombieAccounts
bool MPDbSQLite::killZombieAccounts(uint64_t nMaxLifeTimeExpectancyInMillis)
{
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;
	uint64_t nEpochTimeFromWhichAZombieMustDie;

	TSK_DEBUG_INFO("MPDbSQLite::killZombieAccounts(%llu)", nMaxLifeTimeExpectancyInMillis);

	ret = dbTransacExec("BEGIN IMMEDIATE TRANSACTION;");

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_DeleteZombies, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_DeleteZombies, ret);
		goto bail;
	}
	
	nEpochTimeFromWhichAZombieMustDie = (tsk_time_epoch() - nMaxLifeTimeExpectancyInMillis);
	if((ret = sqlite3_bind_int64(pCompiledStatement, 1, nEpochTimeFromWhichAZombieMustDie)) == SQLITE_OK)
	{
		ret = MPDbSQLite::dbStep(pCompiledStatement);
	}
	sqlite3_finalize(pCompiledStatement);

bail:
	ret = dbTransacExec("COMMIT TRANSACTION;");
	return (ret == SQLITE_DONE);
}

// MPDb::addAccount
bool MPDbSQLite::addAccount(MPObjectWrapper<MPDbAccount*> oAccount)
{
	assert(*oAccount);
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	ret = dbTransacExec("BEGIN IMMEDIATE TRANSACTION;");

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_InsertAccount, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_InsertAccount, ret);
		goto bail;
	}

	ret += sqlite3_bind_int(pCompiledStatement, 1, oAccount->getSoftVersion());
	ret += sqlite3_bind_int(pCompiledStatement, 2, oAccount->getDatabaseVersion());
	ret += sqlite3_bind_text(pCompiledStatement, 3, oAccount->getName(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 4, oAccount->getEmail(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 5, oAccount->getPassword(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 6, oAccount->getAuthToken(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_int(pCompiledStatement, 7, oAccount->isActivated() ? 1 : 0);
	ret += sqlite3_bind_text(pCompiledStatement, 8, oAccount->getActivationCode(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_int64(pCompiledStatement, 9, oAccount->getEpoch());

	if(ret == SQLITE_OK)
	{
		ret = MPDbSQLite::dbStep(pCompiledStatement);
	}
	sqlite3_finalize(pCompiledStatement);


bail:
	dbTransacExec("COMMIT TRANSACTION;");
	return (ret == SQLITE_DONE);
}

// MPDb::addAccountSip
bool MPDbSQLite::addAccountSip(MPObjectWrapper<MPDbAccountSip*> oAccountSip)
{
	assert(*oAccountSip);
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	ret = dbTransacExec("BEGIN IMMEDIATE TRANSACTION;");

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_InsertAccountSip, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_InsertAccountSip, ret);
		goto bail;
	}

	ret += sqlite3_bind_text(pCompiledStatement, 1, oAccountSip->getAddress64(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_int64(pCompiledStatement, 2, oAccountSip->getAccountId());

	if(ret == SQLITE_OK)
	{
		ret = MPDbSQLite::dbStep(pCompiledStatement);
	}
	sqlite3_finalize(pCompiledStatement);

bail:
	dbTransacExec("COMMIT TRANSACTION;");
	return (ret == SQLITE_DONE);
}

bool MPDbSQLite::addAccountSipCaller(MPObjectWrapper<MPDbAccountSipCaller*> oAccountSipCaller)
{
	assert(*oAccountSipCaller);
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	ret = dbTransacExec("BEGIN IMMEDIATE TRANSACTION;");

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_InsertAccountSipCaller, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_InsertAccountSipCaller, ret);
		goto bail;
	}

	ret += sqlite3_bind_text(pCompiledStatement, 1, oAccountSipCaller->getDisplayName(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 2, oAccountSipCaller->getImpu(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 3, oAccountSipCaller->getImpi(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 4, oAccountSipCaller->getRealm(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_text(pCompiledStatement, 5, oAccountSipCaller->getHa1(), -1, SQLITE_TRANSIENT);
	ret += sqlite3_bind_int64(pCompiledStatement, 6, oAccountSipCaller->getAccountSipId());

	if(ret == SQLITE_OK)
	{
		ret = MPDbSQLite::dbStep(pCompiledStatement);
	}
	sqlite3_finalize(pCompiledStatement);

bail:
	dbTransacExec("COMMIT TRANSACTION;");
	return (ret == SQLITE_DONE);
}


// MPDb::selectAccountByEmail
MPObjectWrapper<MPDbAccount*> MPDbSQLite::selectAccountByEmail(const char* pcEmail)
{
	assert(pcEmail);
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_SelectAccountByEmail, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_SelectAccountByEmail, ret);
		return NULL;
	}

	if((ret = sqlite3_bind_text(pCompiledStatement, 1, pcEmail, -1, SQLITE_TRANSIENT)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to bind text for sqlQuery = [%s] error code = %d", MPDb::sSQL_SelectAccountByEmail, ret);
		sqlite3_finalize(pCompiledStatement);
		return NULL;
	}

	if((ret = MPDbSQLite::dbStep(pCompiledStatement)) == SQLITE_ROW)
	{
		int64_t nId = sqlite3_column_int64(pCompiledStatement, 0);
		int nSoftVersion = sqlite3_column_int(pCompiledStatement, 1);
		int nDatabaseVersion = sqlite3_column_int(pCompiledStatement, 2);
		const char* pcName = (const char*)sqlite3_column_text(pCompiledStatement, 3);
		const char* pcEmail = (const char*)sqlite3_column_text(pCompiledStatement, 4);
		const char* pcPassword = (const char*)sqlite3_column_text(pCompiledStatement, 5);
		const char* pctAuthToken = (const char*)sqlite3_column_text(pCompiledStatement, 6);
		bool bActivated = (sqlite3_column_int(pCompiledStatement, 7) != 0);
		const char* pcActivationCode = (const char*)sqlite3_column_text(pCompiledStatement, 8);
		
		return new MPDbAccount(nId, nSoftVersion, nDatabaseVersion, pcName, pcEmail, pcPassword, pctAuthToken, bActivated, pcActivationCode);
	}

	return NULL;
}

// MPDb::selectAccountSipByAddress
MPObjectWrapper<MPDbAccountSip*> MPDbSQLite::selectAccountSipByAddress(const char* pcAddress)
{
	assert(pcAddress);
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_SelectAccountSipByAddress, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_SelectAccountSipByAddress, ret);
		return NULL;
	}

	if((ret = sqlite3_bind_text(pCompiledStatement, 1, pcAddress, -1, SQLITE_TRANSIENT)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to bind text for sqlQuery = [%s] error code = %d", MPDb::sSQL_SelectAccountSipByAddress, ret);
		sqlite3_finalize(pCompiledStatement);
		return NULL;
	}

	if((ret = MPDbSQLite::dbStep(pCompiledStatement)) == SQLITE_ROW)
	{
		int64_t nId = sqlite3_column_int64(pCompiledStatement, 0);
		const char* pcAddress = (const char*)sqlite3_column_text(pCompiledStatement, 1);
		int64_t nAccountId = sqlite3_column_int64(pCompiledStatement, 0);
		
		return new MPDbAccountSip(nId, pcAddress, nAccountId);
	}

	return NULL;
}

// MPDb::selectAccountSipByAccountId
MPObjectWrapper<MPDbAccountSip*> MPDbSQLite::selectAccountSipByAccountId(int64_t nAccountId)
{
	assert(nAccountId != MPDB_ACCOUNT_INVALID_ID);
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_SelectAccountSipByAccountId, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_SelectAccountSipByAccountId, ret);
		return NULL;
	}

	if((ret = sqlite3_bind_int64(pCompiledStatement, 1, nAccountId)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to bind text for sqlQuery = [%s] error code = %d", MPDb::sSQL_SelectAccountSipByAccountId, ret);
		sqlite3_finalize(pCompiledStatement);
		return NULL;
	}

	if((ret = MPDbSQLite::dbStep(pCompiledStatement)) == SQLITE_ROW)
	{
		int64_t nId = sqlite3_column_int64(pCompiledStatement, 0);
		const char* pcAddress = (const char*)sqlite3_column_text(pCompiledStatement, 1);
		int64_t nAccountId = sqlite3_column_int64(pCompiledStatement, 0);
		
		return new MPDbAccountSip(nId, pcAddress, nAccountId);
	}

	return NULL;
}

MPObjectWrapper<MPDbAccountSip*> MPDbSQLite::selectAccountSipByAccountIdAndAddress(int64_t nAccountId, const char* pcAddress)
{
	assert(nAccountId != MPDB_ACCOUNT_INVALID_ID && pcAddress);
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_SelectAccountSipByAccountIdAndAddress, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_SelectAccountSipByAccountIdAndAddress, ret);
		return NULL;
	}

	if((ret = sqlite3_bind_int64(pCompiledStatement, 1, nAccountId)) != SQLITE_OK || (ret = sqlite3_bind_text(pCompiledStatement, 2, pcAddress, -1, SQLITE_TRANSIENT)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to bind text for sqlQuery = [%s] error code = %d", MPDb::sSQL_SelectAccountSipByAccountIdAndAddress, ret);
		sqlite3_finalize(pCompiledStatement);
		return NULL;
	}

	if((ret = MPDbSQLite::dbStep(pCompiledStatement)) == SQLITE_ROW)
	{
		int64_t nId = sqlite3_column_int64(pCompiledStatement, 0);
		const char* pcAddress = (const char*)sqlite3_column_text(pCompiledStatement, 1);
		int64_t nAccountId = sqlite3_column_int64(pCompiledStatement, 0);
		
		return new MPDbAccountSip(nId, pcAddress, nAccountId);
	}

	return NULL;
}

MPObjectWrapper<MPDbAccountSipCaller*> MPDbSQLite::selectAccountSipCallerById(int64_t nId)
{
	return selectAccountSipCaller(nId, false);
}

MPObjectWrapper<MPDbAccountSipCaller*> MPDbSQLite::selectAccountSipCallerByAccountSipId(int64_t nAccountSipId)
{
	return selectAccountSipCaller(nAccountSipId, true);
}

bool MPDbSQLite::deleteAccountSipById(uint64_t id)
{
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_DeleteAccountSip, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_DeleteAccountSip, ret);
		return false;
	}
	
	if((ret = sqlite3_bind_int64(pCompiledStatement, 1, id)) == SQLITE_OK)
	{
		ret = MPDbSQLite::dbStep(pCompiledStatement);
	}
	sqlite3_finalize(pCompiledStatement);

	return (ret == SQLITE_DONE);
}

bool MPDbSQLite::deleteAccountSipCallerById(uint64_t id)
{
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;

	if((ret = sqlite3_prepare_v2(m_pEngine, MPDb::sSQL_DeleteAccountSipCaller, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", MPDb::sSQL_DeleteAccountSipCaller, ret);
		return false;
	}
	
	if((ret = sqlite3_bind_int64(pCompiledStatement, 1, id)) == SQLITE_OK)
	{
		ret = MPDbSQLite::dbStep(pCompiledStatement);
	}
	sqlite3_finalize(pCompiledStatement);

	return (ret == SQLITE_DONE);
}

// MPDb::open
bool MPDbSQLite::close()
{
	if(m_pEngine)
	{
		sqlite3_close(m_pEngine);
		m_pEngine = NULL;
	}
	m_bOpened = false;
	return true;
}

// private
MPObjectWrapper<MPDbAccountSipCaller*> MPDbSQLite::selectAccountSipCaller(int64_t nId, bool bByAccountSipId)
{
	assert(isOpened());

	sqlite3_stmt* pCompiledStatement = NULL;
	int ret;
	const char* pSql = bByAccountSipId ? MPDb::sSQL_SelectAccountSipCallerByAccountSipId : MPDb::sSQL_SelectAccountSipCallerById;

	if((ret = sqlite3_prepare_v2(m_pEngine, pSql, -1, &pCompiledStatement, NULL)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to prepare sqlQuery = [%s] error code = %d", pSql, ret);
		return NULL;
	}

	if((ret = sqlite3_bind_int64(pCompiledStatement, 1, nId)) != SQLITE_OK)
	{
		TSK_DEBUG_ERROR("Failed to bind text for sqlQuery = [%s] error code = %d", pSql, ret);
		sqlite3_finalize(pCompiledStatement);
		return NULL;
	}
	
	if((ret = MPDbSQLite::dbStep(pCompiledStatement)) == SQLITE_ROW)
	{
		int64_t nAccountSipCallerId = sqlite3_column_int64(pCompiledStatement, 0);
		const char* pcDisplayName = (const char*)sqlite3_column_text(pCompiledStatement, 1);
		const char* pcImpu = (const char*)sqlite3_column_text(pCompiledStatement, 2);
		const char* pcImpi = (const char*)sqlite3_column_text(pCompiledStatement, 3);
		const char* pcRealm = (const char*)sqlite3_column_text(pCompiledStatement, 4);
		const char* pcHa1 = (const char*)sqlite3_column_text(pCompiledStatement, 5);
		int64_t nAccountSipId = sqlite3_column_int64(pCompiledStatement, 6);
		
		return new MPDbAccountSipCaller(nAccountSipCallerId, pcDisplayName, pcImpu, pcImpi, pcRealm, pcHa1, nAccountSipId);
	}

	return NULL;
}

// private
int MPDbSQLite::dbTransacExec(const char* pcQuery)
{
	assert(isOpened() && pcQuery);

	int ret;
	uint64_t nTimeOut = 200;

	while(nTimeOut < (200 << 5) && (ret = sqlite3_exec(m_pEngine, pcQuery, 0, 0, 0)) == SQLITE_BUSY)
	{
		TSK_DEBUG_INFO("dbTransacExec::SQLITE_BUSY");
		// 200, 400, 800, 1600, 3200
		tsk_thread_sleep(nTimeOut);
		nTimeOut <<= 1;
	}

	return ret;
}

// private
int MPDbSQLite::dbStep(void* pCompiledStatement)
{
	assert(pCompiledStatement);

	int ret;
	uint64_t nTimeOut = 200;

	while(nTimeOut < (200 << 5) && (ret = sqlite3_step((struct sqlite3_stmt*)pCompiledStatement)) == SQLITE_BUSY)
	{
		TSK_DEBUG_INFO("dbStep::SQLITE_BUSY");
		// 200, 400, 800, 1600, 3200
		tsk_thread_sleep(nTimeOut);
		nTimeOut <<= 1;
	}

	return ret;
}

}//namespace