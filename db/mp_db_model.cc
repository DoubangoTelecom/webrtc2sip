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
#include "mp_db_model.h"

#include "tsk_memory.h"
#include "tsk_debug.h"
#include "tsk_base64.h"
#include "tsk_md5.h"
#include "tsk_time.h"
#include "tsk_uuid.h"
#include "tsk_string.h"

#include <assert.h>

namespace webrtc2sip {

//
//	MPDbAccountSip
//
MPDbAccountSip::MPDbAccountSip(int64_t nId, const char* pcAddress64, int64_t nAccountId)
{
	m_nId = nId;
	m_pAddress64 = tsk_strdup(pcAddress64);
	m_nAccountId = nAccountId;
}

MPDbAccountSip::~MPDbAccountSip()
{
	TSK_FREE(m_pAddress64);
}


//
//	MPDbAccountSipCaller
//
MPDbAccountSipCaller::MPDbAccountSipCaller(int64_t nId, const char* pcDisplayName, const char* pcImpu, const char* pcImpi, const char* pcRealm, const char* pcHa1, int64_t nAccountSipId)
{
	m_nId = nId;
	m_pDisplayName = tsk_strdup(pcDisplayName);
	m_pImpu = tsk_strdup(pcImpu);
	m_pImpi = tsk_strdup(pcImpi);
	m_pRealm = tsk_strdup(pcRealm);
	m_pHa1 = tsk_strdup(pcHa1);
	m_nAccountSipId = nAccountSipId;
}

MPDbAccountSipCaller::~MPDbAccountSipCaller()
{
	TSK_FREE(m_pDisplayName);
	TSK_FREE(m_pImpu);
	TSK_FREE(m_pImpi);
	TSK_FREE(m_pRealm);
	TSK_FREE(m_pHa1);
}


//
//	MPDbAccount
//
MPDbAccount::MPDbAccount(int64_t nId, int32_t nSoftVersion, int32_t nDatabaseVersion, const char* pcName, const char* pcEmail, const char* pcPassword /*= NULL*/, const char* pcAuthToken /*= NULL*/, bool bActivated /*= false*/, const char* pcActivationCode /*= NULL*/, uint64_t nEpoch /*= 0*/)
: m_nId(nId)
, m_nSoftVersion(nSoftVersion)
, m_nDatabaseVersion(nDatabaseVersion)
, m_pName(NULL)
, m_pEmail(NULL)
, m_pPassword(NULL)
, m_pAuthToken(NULL)
, m_bActivated(bActivated)
{
	m_pName = tsk_strdup(pcName);
	m_pEmail = tsk_strdup(pcEmail);
	m_pPassword = tsk_strdup(pcPassword ? pcPassword : MPDbAccount::buildRandomPassword().c_str());
	m_pAuthToken = tsk_strdup(pcAuthToken ? pcAuthToken : MPDbAccount::buildAuthToken(m_pPassword, m_pEmail).c_str());
	m_pActivationCode = tsk_strdup(pcActivationCode ? pcActivationCode : MPDbAccount::buildRandomActivationCode().c_str());
	m_nEpoch = nEpoch ? nEpoch : tsk_time_epoch();
}

MPDbAccount::~MPDbAccount()
{
	TSK_FREE(m_pName);
	TSK_FREE(m_pEmail);
	TSK_FREE(m_pPassword);
	TSK_FREE(m_pAuthToken);
	TSK_FREE(m_pActivationCode);
}

MPObjectWrapper<MPData*> MPDbAccount::serialize()
{
	char* pStr = NULL;
	return NULL;
}

std::string MPDbAccount::buildRandomActivationCode()
{
	tsk_uuidstring_t uuid;
	tsk_uuidgenerate(&uuid);
	return std::string((const char*)&uuid[0]);
}

std::string MPDbAccount::buildRandomPassword()
{
	static const char __password_chars[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'k', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
									'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'K', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
								'0','1', '2', '3', '4', '5', '6', '7', '8', '9'};
	static const size_t __password_chars_count = sizeof(__password_chars);
	char password[10] = {'\0'};
	for(int i = 0; i < sizeof(password) - 1; ++i)
	{
		password[i] = __password_chars[(rand() ^ tsk_time_now()) % __password_chars_count];
	}
	return std::string(password);
}

std::string MPDbAccount::buildAuthToken(const char* pcPassword, const char* pcEmail)
{
	assert(pcPassword && pcEmail);
	char* authTokenStr = tsk_null;
	tsk_md5string_t authToken;
	tsk_sprintf(&authTokenStr, "%s:%s:click2call.org", pcPassword, pcEmail);
	tsk_md5compute(authTokenStr, tsk_strlen(authTokenStr), &authToken);
	TSK_FREE(authTokenStr);
	return std::string((const char*)&authToken[0]);
}


}//namespace
