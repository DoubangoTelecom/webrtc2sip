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
#include "mp_db.h"
#include "sqlite/mp_db_sqlite.h"

#include "tsk_string.h"
#include "tsk_memory.h"
#include "tsk_debug.h"

#include <assert.h>

namespace webrtc2sip {

const char* MPDb::sSQL = 
"CREATE TABLE IF NOT EXISTS account("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "softVersion INTEGER,"
        "databaseVersion INTEGER,"
		"name TEXT NOT NULL,"
		"email TEXT NOT NULL UNIQUE,"
		"password TEXT,"
		"auth_token TEXT,"
		"activated TINYINT(1),"
		"activation_code TEXT,"
		"epoch BIGINT"
		");"
		"CREATE INDEX IF NOT EXISTS email_idx ON account (email);"
""
"CREATE TABLE IF NOT EXISTS account_sip("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"address TEXT NOT NULL,"
		"account_id INTEGER,"
		"FOREIGN KEY(account_id) REFERENCES account(id) ON DELETE CASCADE"
		");"
		"CREATE INDEX IF NOT EXISTS address_idx ON account_sip (address);"
""
"CREATE TABLE IF NOT EXISTS account_sip_caller("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"display_name TEXT,"
		"impu TEXT NOT NULL,"
		"impi TEXT NOT NULL,"
		"realm TEXT NOT NULL,"
		"ha1 TEXT NOT NULL,"
		"account_sip_id INTEGER,"
		"FOREIGN KEY(account_sip_id) REFERENCES account_sip(id) ON DELETE CASCADE"
		");"
;

const char* MPDb::sSQL_DeleteZombies = "DELETE FROM account WHERE epoch < ? AND activated = 0";
const char* MPDb::sSQL_InsertAccount = "INSERT INTO account("
		"softVersion,"
		"databaseVersion,"
		"name,"
		"email,"
		"password,"
		"auth_token,"
		"activated,"
		"activation_code,"
		"epoch"
		")"
		" VALUES(?,?,?,?,?,?,?,?,?)";
const char* MPDb::sSQL_UpdateAccountById = "UPDATE account SET "
		"softVersion = ?,"
		"databaseVersion = ?,"
		"name = ?,"
		"email = ?,"
		"password = ?,"
		"auth_token = ?,"
		"activated = ?,"
		"activation_code = ?,"
		"epoch = ?"
		" WHERE id = ?";

const char* MPDb::sSQL_SelectAccountByEmail = "SELECT "
		"id,"
		"softVersion,"
		"databaseVersion,"
		"name,"
		"email,"
		"password,"
		"auth_token,"
		"activated,"
		"activation_code,"
		"epoch"
		" FROM account WHERE email = ?";


const char* MPDb::sSQL_DeleteAccountSip = "DELETE FROM account_sip WHERE id = ?";
const char* MPDb::sSQL_InsertAccountSip = "INSERT INTO account_sip(address,account_id) VALUES(?,?)";
const char* MPDb::sSQL_SelectAccountSipByAddress = "SELECT id,address,account_id FROM account_sip WHERE address = ?";
const char* MPDb::sSQL_SelectAccountSipByAccountId = "SELECT id,address,account_id FROM account_sip WHERE account_id = ?";
const char* MPDb::sSQL_SelectAccountSipByAccountIdAndAddress = "SELECT id,address,account_id FROM account_sip WHERE account_id = ? AND address = ?";

const char* MPDb::sSQL_DeleteAccountSipCaller = "DELETE FROM account_sip_caller WHERE id = ?";
const char* MPDb::sSQL_InsertAccountSipCaller = "INSERT INTO account_sip_caller("
		"display_name,"
		"impu,"
		"impi,"
		"realm,"
		"ha1,"
		"account_sip_id"
		")"
		" VALUES(?,?,?,?,?,?)";
const char* MPDb::sSQL_SelectAccountSipCallerById = "SELECT "
		"id,"
		"display_name,"
		"impu,"
		"impi,"
		"realm,"
		"ha1,"
		"account_sip_id"
		" FROM account_sip_caller WHERE id = ?";
const char* MPDb::sSQL_SelectAccountSipCallerByAccountSipId = "SELECT "
		"id,"
		"display_name,"
		"impu,"
		"impi,"
		"realm,"
		"ha1,"
		"account_sip_id"
		" FROM account_sip_caller WHERE account_sip_id = ?";


MPDb::MPDb(MPDbType_t eType, const char* pcConnectionInfo)
{
	m_eType = eType;
	m_pConnectionInfo = tsk_strdup(pcConnectionInfo);
}

MPDb::~MPDb()
{
	TSK_FREE(m_pConnectionInfo);
}

MPObjectWrapper<MPDb*> MPDb::New(MPDbType_t eType, const char* pcConnectionInfo)
{
	switch(eType)
	{
	case MPDbType_SQLite:
		{
			return new MPDbSQLite(pcConnectionInfo);
		}
	}
	
	TSK_DEBUG_ERROR("Cannot create database with type = %d", eType);

	return NULL;
}

}//namespace