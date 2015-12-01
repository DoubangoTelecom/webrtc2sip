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
#include "mp_c2c.h"
#include "db/mp_db_model.h"

#include "jsoncpp/json/json.h"

#include "tsk_string.h"
#include "tsk_memory.h"

#include "tinyhttp.h"

#include <assert.h>

/* max number of accounts to be added before killing zombies. They will also be killed at startup */
#define kMaxNewAccountBeforeKillingZombies	100
/* maximum number of milisenconds a zombie is allowed in the database. A zombie is an inactivated account */
#define kZombieMaxLifeTimeExpectancy (24 * 60 * 60 * 1000 * 2) /* 2 days */

/* min size of a stream chunck to form a valid HTTP message */
#define kStreamChunckMinSize 0x32
#define kHttpMethodOptions "OPTIONS"
#define kHttpMethodPost "POST"

#define kJsonContentType "application/json"

#define kJsonField_Action "action"
#define kJsonField_Name "name"
#define kJsonField_Email "email"
#define kJsonField_Code "code"
#define kJsonField_AuthToken "auth_token"
#define kJsonField_Sip "sip"
#define kJsonField_Address "address"
#define kJsonField_Id "id"
#define kJsonField_DisplayName "display_name"
#define kJsonField_Impu "impu"
#define kJsonField_Impi "impi"
#define kJsonField_Realm "realm"
#define kJsonField_AccountSipCallerId "account_sip_id"
#define kJsonField_Ha1 "ha1"
#define kJsonField_CaptchaValue "captcha_value"
                

#define kJsonValue_ActionReq_AccountAdd "req_account_add"
#define kJsonValue_ActionReq_AccountSipAdd "req_account_sip_add"
#define kJsonValue_ActionReq_AccountSipCallerAdd "req_account_sip_caller_add"
#define kJsonValue_ActionReq_AccountSipDelete "req_account_sip_delete"
#define kJsonValue_ActionReq_AccountSipCallerDelete "req_account_sip_caller_delete"
#define kJsonValue_ActionReq_AccountActivate "req_account_activate"
#define kJsonValue_ActionReq_AccountInfo "req_account_info"

#define kJsonValue_ActionResp_AccountAdd "resp_account_add"
#define kJsonValue_ActionResp_AccountSipAdd "resp_account_sip_add"
#define kJsonValue_ActionResp_AccountSipCallerAdd "resp_account_sip_caller_add"
#define kJsonValue_ActionResp_AccountSipDelete "resp_account_sip_delete"
#define kJsonValue_ActionResp_AccountSipCallerDelete "resp_account_sip_caller_delete"
#define kJsonValue_ActionResp_AccountActivate "resp_account_activate"
#define kJsonValue_ActionResp_AccountInfo "resp_account_info"

#define kJsonContent_ActionResp_AccountAdd "{\"action\": \"" kJsonValue_ActionResp_AccountAdd "\"}"
#define kJsonContent_ActionResp_AccountSipAdd "{\"action\": \"" kJsonValue_ActionResp_AccountSipAdd "\"}"
#define kJsonContent_ActionResp_AccountSipCallerAdd "{\"action\": \"" kJsonValue_ActionResp_AccountSipCallerAdd "\"}"
#define kJsonContent_ActionResp_AccountActivate "{\"action\": \"" kJsonValue_ActionResp_AccountActivate "\"}"
#define kJsonContent_ActionResp_AccountInfo "{\"action\": \"" kJsonValue_ActionResp_AccountInfo "\"}"
#define kJsonContent_ActionResp_AccountSipDelete "{\"action\": \"" kJsonValue_ActionResp_AccountSipDelete "\"}"
#define kJsonContent_ActionResp_AccountSipCallerDelete "{\"action\": \"" kJsonValue_ActionResp_AccountSipCallerDelete "\"}"

#define kMPC2CResultCode_Success				200
#define kMPC2CResultCode_Accepted				202
#define kMPC2CResultCode_Unauthorized			403
#define kMPC2CResultCode_NotFound				404
#define kMPC2CResultCode_ParsingFailed			420
#define kMPC2CResultCode_InvalidDataType		483
#define kMPC2CResultCode_InvalidData			450
#define kMPC2CResultCode_InternalError			603
#define kMPC2CResultCode_InvalidHttpDomain		kMPC2CResultCode_Unauthorized

#define kMPC2CResultPhrase_Success				"OK"
#define kMPC2CResultPhrase_Accepted				"Accepted"
#define kMPC2CResultPhrase_CaptchaValidation	"Captcha validation"
#define kMPC2CResultPhrase_Unauthorized			"Unauthorized"
#define kMPC2CResultPhrase_NotFound				"Not Found"
#define kMPC2CResultPhrase_ParsingFailed		"Parsing failed"
#define kMPC2CResultPhrase_InvalidDataType		"Invalid data type"
#define kMPC2CResultPhrase_InternalError		"Internal Error"
#define kMPC2CResultPhrase_InvalidHttpDomain	"Invalid http domain"

#define kMailWelcomeSubject "Your click2dial.org registration info"


// Access-Control-Allow-Headers, wildcard not allowed: http://www.w3.org/TR/cors/#access-control-allow-headers-response-header
// Connection should not be "Keep-Alive" to avoid ghost sockets (do not let the browser control when the socket have to be closed)
#define kHttpOptionsResponse \
	"HTTP/1.1 200 OK\r\n" \
	"Server: webrtc2sip Media Server " WEBRTC2SIP_VERSION_STRING "\r\n" \
	"Access-Control-Allow-Origin: *\r\n" \
	"Access-Control-Allow-Headers: Content-Type,Connection\r\n" \
	"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n" \
	"Content-Length: 0\r\n" \
	"Content-Type: application/json\r\n" \
	"Connection: Close\r\n" \
	"\r\n"
#define kHttpResponse(code, phrase) \
	"HTTP/1.1 " #code " " phrase "\r\n" \
	"Server: webrtc2sip Media Server " WEBRTC2SIP_VERSION_STRING "\r\n" \
	"Access-Control-Allow-Origin: *\r\n" \
	"Content-Length: 0\r\n" \
	"Connection: Close\r\n" \
	"\r\n"


namespace webrtc2sip {

/* number of accounts added since the gateway started */
uint64_t MPC2CTransport::s_nAccountsCount  = 0;

class MPC2CTransportCallback;

static bool isJsonContentType(const thttp_message_t *pcHttpMessage);
static const char* getHttpContentType(const thttp_message_t *pcHttpMessage);


//
//	MPC2CResult
// 

MPC2CResult::MPC2CResult(unsigned short nCode, const char* pcPhrase, const void* pcDataPtr, size_t nDataSize)
: m_pDataPtr(NULL)
, m_nDataSize(0)
{
	m_nCode = nCode;
	m_pPhrase = tsk_strdup(pcPhrase);
	if(pcDataPtr && nDataSize)
	{
		if((m_pDataPtr = tsk_malloc(nDataSize)))
		{
			memcpy(m_pDataPtr, pcDataPtr, nDataSize);
			m_nDataSize = nDataSize;
		}
	}
}

MPC2CResult::~MPC2CResult()
{
	TSK_FREE(m_pPhrase);
	TSK_FREE(m_pDataPtr);
}

//
//	MPC2CTransport
//

MPC2CTransport::MPC2CTransport(bool isSecure, const char* pcLocalIP, unsigned short nLocalPort)
: MPNetTransport(isSecure ? MPNetTransporType_TLS : MPNetTransporType_TCP, pcLocalIP, nLocalPort)
{
	m_oCallback = new MPC2CTransportCallback(this);
	m_oRecaptchaValidationCallback = new MPC2CRecaptchaValidationCallback(this);
	setCallback(*m_oCallback);
}

MPC2CTransport::~MPC2CTransport()
{
	setCallback(NULL);
	if (m_oRecaptchaTransport) {
		m_oRecaptchaTransport->setValidationCallback(NULL);
	}
}

void MPC2CTransport::setDb(MPObjectWrapper<MPDb*> oDb)
{
	m_oDb = oDb;
}

void MPC2CTransport::setMailTransport(MPObjectWrapper<MPMailTransport*> oMailTransport)
{
	m_oMailTransport = oMailTransport;
}

void MPC2CTransport::setRecaptchaTransport(MPObjectWrapper<MPRecaptchaTransport*> oRecaptchaTransport)
{
	m_oRecaptchaTransport = oRecaptchaTransport;
	if (m_oRecaptchaTransport) {
		m_oRecaptchaTransport->setValidationCallback(*m_oRecaptchaValidationCallback);
	}
}

MPObjectWrapper<MPData*> MPC2CTransport::serializeAccount(const char* pcActionId, MPObjectWrapper<MPDbAccount*> oAccount)const
{
	assert(pcActionId && *oAccount);

	char* pStr = NULL;
	tsk_sprintf(&pStr, "{\"action\":\"%s\",\"name\":\"%s\"", pcActionId, oAccount->getName());
	MPObjectWrapper<MPDbAccountSip*> oAccountSip = m_oDb->selectAccountSipByAccountId(oAccount->getId());
	if(oAccountSip)
	{
		char* pSipAddress = NULL;
		MPObjectWrapper<MPDbAccountSipCaller*> oAccountSipCaller = m_oDb->selectAccountSipCallerByAccountSipId(oAccountSip->getId());
		tsk_base64_decode((const uint8_t*)oAccountSip->getAddress64(), tsk_strlen(oAccountSip->getAddress64()), &pSipAddress);
		tsk_strcat_2(&pStr, ",\"sip_accounts\":[{\"id\":%llu,\"address\":\"%s\"", oAccountSip->getId(), pSipAddress);
		if(oAccountSipCaller)
		{
			tsk_strcat_2(&pStr, ",\"sip_account_callers\":[{\"id\":%llu,\"display_name\":\"%s\",\"impu\":\"%s\",\"impi\":\"%s\",\"realm\":\"%s\"}]", 
					oAccountSipCaller->getId(), 
					oAccountSipCaller->getDisplayName(),
					oAccountSipCaller->getImpu(),
					oAccountSipCaller->getImpi(),
					oAccountSipCaller->getRealm()
				);
		}
		tsk_strcat_2(&pStr, "}]");
		TSK_FREE(pSipAddress);
	}
	tsk_strcat(&pStr,  "}");
	return new MPData((void**)&pStr, tsk_strlen(pStr));
}


// FIXME: factorize
MPObjectWrapper<MPC2CResult*> MPC2CTransport::handleJsonContent(MPNetFd nFd, const void* pcDataPtr, size_t nDataSize)const
{
	if(!pcDataPtr || !nDataSize)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return new MPC2CResult(kMPC2CResultCode_InternalError, "Invalid parameter");
	}

	if(!m_oDb)
	{
		TSK_DEBUG_ERROR("No valid database engine could be found");
		return new MPC2CResult(kMPC2CResultCode_InternalError, "No valid database engine could be found");
	}

	#define MP_JSON_GET(fieldParent, fieldVarName, fieldName, typeTestFun, couldBeNull) \
		if(!fieldParent.isObject()){ \
			TSK_DEBUG_ERROR("JSON '%s' not an object", (fieldName)); \
			return new MPC2CResult(kMPC2CResultCode_InvalidDataType, kMPC2CResultPhrase_InvalidDataType); \
		} \
		const Json::Value fieldVarName = (fieldParent)[(fieldName)]; \
		if((fieldVarName).isNull() && !(couldBeNull)) \
		{ \
			TSK_DEBUG_ERROR("JSON '%s' is null", (fieldName)); \
			return new MPC2CResult(kMPC2CResultCode_InvalidDataType, "Required field is missing"); \
		} \
		if(!(fieldVarName).typeTestFun()) \
		{ \
			TSK_DEBUG_ERROR("JSON '%s' has invalid type", (fieldName)); \
			return new MPC2CResult(kMPC2CResultCode_InvalidDataType, kMPC2CResultPhrase_InvalidDataType); \
		}
		

	Json::Value root;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse((const char*)pcDataPtr, (((const char*)pcDataPtr) + nDataSize), root);
	if (!parsingSuccessful)
	{
		TSK_DEBUG_ERROR("Failed to parse JSON content: %.*s", (int)nDataSize, pcDataPtr);
		return new MPC2CResult(kMPC2CResultCode_ParsingFailed, kMPC2CResultPhrase_ParsingFailed);
	}

	// JSON::action
	MP_JSON_GET(root, action, kJsonField_Action, isString, false);

	// JSON::action=='req_account_add'
	if(tsk_striequals(action.asString().c_str(), kJsonValue_ActionReq_AccountAdd))
	{
		// FIXME
		// JSON::name
		MP_JSON_GET(root, name, kJsonField_Name, isString, false);
		// JSON::email
		MP_JSON_GET(root, email, kJsonField_Email, isString, false);

		if (m_oRecaptchaTransport)
		{
			TSK_DEBUG_INFO("reCAPTCHA enabled. Delaying add account and activation mail until verification is done");
			// JSON::captcha_value
			MP_JSON_GET(root, CaptchaValue, kJsonField_CaptchaValue, isString, false);
			if (m_oRecaptchaTransport->validate(nFd, name.asCString(), email.asCString(), CaptchaValue.asCString()))
			{
				return new MPC2CResult(kMPC2CResultCode_Accepted, kMPC2CResultPhrase_CaptchaValidation);
			}
			else
			{
				return new MPC2CResult(kMPC2CResultCode_InternalError, kMPC2CResultPhrase_InternalError);
			}
		}
		else
		{
			return addAccountAndSendActivationMail(email.asCString(), name.asCString());
		}
	}
	// JSON::action=='req_account_sip_add'
	else if(tsk_striequals(action.asString().c_str(), kJsonValue_ActionReq_AccountSipAdd))
	{
		static const char __pcContentPtr[] = kJsonContent_ActionResp_AccountSipAdd;
		static const size_t __nContentSize = sizeof(__pcContentPtr) - 1/*\0*/;
		
		// JSON::email
		MP_JSON_GET(root, email, kJsonField_Email, isString, true);
		// JSON::auth_token
		MP_JSON_GET(root, authToken, kJsonField_AuthToken, isString, true);
		// JSON::sip
		MP_JSON_GET(root, sip, kJsonField_Sip, isObject, true);
		// JSON::sip::address
		MP_JSON_GET(sip, address, kJsonField_Address, isString, true);

		// find the account
		MPObjectWrapper<MPDbAccount*> oAccount = m_oDb->selectAccountByEmail(email.asString().c_str());
		if(!oAccount)
		{
			TSK_DEBUG_ERROR("Failed to find account with email id = %s", email.asString().c_str());
			return new MPC2CResult(kMPC2CResultCode_NotFound, "Failed to find account", __pcContentPtr, __nContentSize);
		}
		// check whether the account is activated
		if(!oAccount->isActivated())
		{
			TSK_DEBUG_ERROR("Account not activated: Operation not allowed");
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Operation not allowed (Account not activated)", __pcContentPtr, __nContentSize);
		}
		// check authentication token
		if(!tsk_strequals(oAccount->getAuthToken(), authToken.asString().c_str()))
		{
			TSK_DEBUG_ERROR("Authentication token mismatch '%s' <> '%s'", authToken.asString().c_str(), oAccount->getAuthToken());
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Authentication token mismatch", __pcContentPtr, __nContentSize);
		}
		// base64 encode the address
		char* pAddress64 = NULL;
		tsk_base64_encode((const uint8_t*)address.asString().c_str(), address.asString().length(), &pAddress64);
		MPObjectWrapper<MPDbAccountSip*> oAccountSip = new MPDbAccountSip(MPDB_ACCOUNT_SIP_INVALID_ID, pAddress64, oAccount->getId());
		TSK_FREE(pAddress64);

		// add SIP account
		if(!m_oDb->addAccountSip(oAccountSip))
		{
			return new MPC2CResult(kMPC2CResultCode_InternalError, "Failed to add SIP account", __pcContentPtr, __nContentSize);
		}
		MPObjectWrapper<MPData*> oData = serializeAccount(kJsonValue_ActionResp_AccountSipAdd, oAccount);
		return new MPC2CResult(kMPC2CResultCode_Success, "SIP account added", oData->getDataPtr(), oData->getDataSize());
	}
	// JSON::action=='req_account_sip_caller_add'
	else if(tsk_striequals(action.asString().c_str(), kJsonValue_ActionReq_AccountSipCallerAdd))
	{
		static const char __pcContentPtr[] = kJsonContent_ActionResp_AccountSipCallerAdd;
		static const size_t __nContentSize = sizeof(__pcContentPtr) - 1/*\0*/;
		
		// JSON::email
		MP_JSON_GET(root, email, kJsonField_Email, isString, true);
		// JSON::auth_token
		MP_JSON_GET(root, authToken, kJsonField_AuthToken, isString, true);
		// JSON::display_name
		MP_JSON_GET(root, displayName, kJsonField_DisplayName, isString, false);
		// JSON::sip::impu
		MP_JSON_GET(root, impu, kJsonField_Impu, isString, true);
		// JSON::sip::impi
		MP_JSON_GET(root, impi, kJsonField_Impi, isString, true);
		// JSON::sip::realm
		MP_JSON_GET(root, realm, kJsonField_Realm, isString, true);
		// JSON::sip::account_sip_id
		MP_JSON_GET(root, accountSipId, kJsonField_AccountSipCallerId, isIntegral, true);
		// JSON::sip::ha1
		MP_JSON_GET(root, ha1, kJsonField_Ha1, isString, true);
		
		// find the account
		MPObjectWrapper<MPDbAccount*> oAccount = m_oDb->selectAccountByEmail(email.asString().c_str());
		if(!oAccount)
		{
			TSK_DEBUG_ERROR("Failed to find account with email id = %s", email.asString().c_str());
			return new MPC2CResult(kMPC2CResultCode_NotFound, "Failed to find account", __pcContentPtr, __nContentSize);
		}
		// check whether the account is activated
		if(!oAccount->isActivated())
		{
			TSK_DEBUG_ERROR("Account not activated: Operation not allowed");
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Operation not allowed (Account not activated)", __pcContentPtr, __nContentSize);
		}
		// check authentication token
		if(!tsk_strequals(oAccount->getAuthToken(), authToken.asString().c_str()))
		{
			TSK_DEBUG_ERROR("Authentication token mismatch '%s' <> '%s'", authToken.asString().c_str(), oAccount->getAuthToken());
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Authentication token mismatch", __pcContentPtr, __nContentSize);
		}

		// add SIP caller account
		MPObjectWrapper<MPDbAccountSipCaller*> oAccountSipCaller = new MPDbAccountSipCaller(
			MPDB_ACCOUNT_SIP_INVALID_ID, 
			displayName.asString().c_str(),
			impu.asString().c_str(),
			impi.asString().c_str(),
			realm.asString().c_str(),
			ha1.asString().c_str(),
			accountSipId.asInt64()
			);
		if(!m_oDb->addAccountSipCaller(oAccountSipCaller))
		{
			return new MPC2CResult(kMPC2CResultCode_InternalError, "Failed to add SIP caller account", __pcContentPtr, __nContentSize);
		}
		MPObjectWrapper<MPData*> oData = serializeAccount(kJsonValue_ActionResp_AccountSipCallerAdd, oAccount);
		return new MPC2CResult(kMPC2CResultCode_Success, "SIP caller account added", oData->getDataPtr(), oData->getDataSize());
	}
	// JSON::action=='req_account_activate'
	else if(tsk_striequals(action.asString().c_str(), kJsonValue_ActionReq_AccountActivate))
	{
		static const char __pcContentPtr[] = kJsonContent_ActionResp_AccountActivate;
		static const size_t __nContentSize = sizeof(__pcContentPtr) - 1/*\0*/;

		// JSON::email
		MP_JSON_GET(root, email, kJsonField_Email, isString, false);
		// JSON::code
		MP_JSON_GET(root, code, kJsonField_Code, isString, false);
		// find the account
		MPObjectWrapper<MPDbAccount*> oAccount = m_oDb->selectAccountByEmail(email.asString().c_str());
		if(!oAccount)
		{
			TSK_DEBUG_ERROR("Failed to find account with email id = %s", email.asString().c_str());
			return new MPC2CResult(kMPC2CResultCode_NotFound, "Failed to find account", __pcContentPtr, __nContentSize);
		}
		// compare activate codes
		if(!tsk_striequals(oAccount->getActivationCode(), code.asString().c_str()))
		{
			TSK_DEBUG_ERROR("Activation code mismatch = '%s' <> '%s'", code.asString().c_str(), oAccount->getActivationCode());
			return new MPC2CResult(kMPC2CResultCode_InvalidDataType, "Activation code mismatch", __pcContentPtr, __nContentSize);
		}
		// update activation state
		if(!oAccount->isActivated())
		{
			oAccount->setActivated(true);
			if(!m_oDb->updateAccount(oAccount))
			{
				return new MPC2CResult(kMPC2CResultCode_InternalError, "Failed to activate account", __pcContentPtr, __nContentSize);
			}
		}
		return new MPC2CResult(kMPC2CResultCode_Success, "Account activated", __pcContentPtr, __nContentSize);
	}
	// JSON::action=='req_account_info'
	else if(tsk_striequals(action.asString().c_str(), kJsonValue_ActionReq_AccountInfo))
	{
		static const char __pcContentPtr[] = kJsonContent_ActionResp_AccountInfo;
		static const size_t __nContentSize = sizeof(__pcContentPtr) - 1/*\0*/;

		// JSON::email
		MP_JSON_GET(root, email, kJsonField_Email, isString, true);
		// JSON::auth_token
		MP_JSON_GET(root, authToken, kJsonField_AuthToken, isString, true);

		// find the account
		MPObjectWrapper<MPDbAccount*> oAccount = m_oDb->selectAccountByEmail(email.asString().c_str());
		if(!oAccount)
		{
			TSK_DEBUG_ERROR("Failed to find account with email id = %s", email.asString().c_str());
			return new MPC2CResult(kMPC2CResultCode_NotFound, "Failed to find account", __pcContentPtr, __nContentSize);
		}
		// check whether the account is activated
		if(!oAccount->isActivated())
		{
			TSK_DEBUG_ERROR("Account not activated: Operation not allowed");
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Operation not allowed (Account not activated)", __pcContentPtr, __nContentSize);
		}
		// check authentication token
		if(!tsk_strequals(oAccount->getAuthToken(), authToken.asString().c_str()))
		{
			TSK_DEBUG_ERROR("Authentication token mismatch '%s' <> '%s'", authToken.asString().c_str(), oAccount->getAuthToken());
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Authentication token mismatch", __pcContentPtr, __nContentSize);
		}

		// serialize account info
		MPObjectWrapper<MPData*> oData = serializeAccount(kJsonValue_ActionResp_AccountInfo, oAccount);
		return new MPC2CResult(kMPC2CResultCode_Success, "Account info", oData->getDataPtr(), oData->getDataSize());
	}
	// JSON::action=='req_account_sip_delete'
	else if(tsk_striequals(action.asString().c_str(), kJsonValue_ActionReq_AccountSipDelete))
	{
		static const char __pcContentPtr[] = kJsonContent_ActionResp_AccountSipDelete;
		static const size_t __nContentSize = sizeof(__pcContentPtr) - 1/*\0*/;
		
		// JSON::email
		MP_JSON_GET(root, email, kJsonField_Email, isString, true);
		// JSON::auth_token
		MP_JSON_GET(root, authToken, kJsonField_AuthToken, isString, true);
		// JSON::id
		MP_JSON_GET(root, id, kJsonField_Id, isIntegral, true);

		
		// find the account
		MPObjectWrapper<MPDbAccount*> oAccount = m_oDb->selectAccountByEmail(email.asString().c_str());
		if(!oAccount)
		{
			TSK_DEBUG_ERROR("Failed to find account with email id = %s", email.asString().c_str());
			return new MPC2CResult(kMPC2CResultCode_NotFound, "Failed to find account", __pcContentPtr, __nContentSize);
		}
		// check whether the account is activated
		if(!oAccount->isActivated())
		{
			TSK_DEBUG_ERROR("Account not activated: Operation not allowed");
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Operation not allowed (Account not activated)", __pcContentPtr, __nContentSize);
		}
		// check authentication token
		if(!tsk_strequals(oAccount->getAuthToken(), authToken.asString().c_str()))
		{
			TSK_DEBUG_ERROR("Authentication token mismatch '%s' <> '%s'", authToken.asString().c_str(), oAccount->getAuthToken());
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Authentication token mismatch", __pcContentPtr, __nContentSize);
		}
		// delete the SIP account/address
		if(!m_oDb->deleteAccountSipById(id.asInt64()))
		{
			return new MPC2CResult(kMPC2CResultCode_InternalError, "Failed to delete SIP address", __pcContentPtr, __nContentSize);
		}
		MPObjectWrapper<MPData*> oData = serializeAccount(kJsonValue_ActionResp_AccountSipDelete, oAccount);
		return new MPC2CResult(kMPC2CResultCode_Success, "SIP address deleted", oData->getDataPtr(), oData->getDataSize());
	}
	// JSON::action=='req_account_sip_caller_delete'
	else if(tsk_striequals(action.asString().c_str(), kJsonValue_ActionReq_AccountSipCallerDelete))
	{
		static const char __pcContentPtr[] = kJsonContent_ActionResp_AccountSipCallerDelete;
		static const size_t __nContentSize = sizeof(__pcContentPtr) - 1/*\0*/;
		
		// JSON::email
		MP_JSON_GET(root, email, kJsonField_Email, isString, true);
		// JSON::auth_token
		MP_JSON_GET(root, authToken, kJsonField_AuthToken, isString, true);
		// JSON::id
		MP_JSON_GET(root, id, kJsonField_Id, isIntegral, true);

		
		// find the account
		MPObjectWrapper<MPDbAccount*> oAccount = m_oDb->selectAccountByEmail(email.asString().c_str());
		if(!oAccount)
		{
			TSK_DEBUG_ERROR("Failed to find account with email id = %s", email.asString().c_str());
			return new MPC2CResult(kMPC2CResultCode_NotFound, "Failed to find account", __pcContentPtr, __nContentSize);
		}
		// check whether the account is activated
		if(!oAccount->isActivated())
		{
			TSK_DEBUG_ERROR("Account not activated: Operation not allowed");
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Operation not allowed (Account not activated)", __pcContentPtr, __nContentSize);
		}
		// check authentication token
		if(!tsk_strequals(oAccount->getAuthToken(), authToken.asString().c_str()))
		{
			TSK_DEBUG_ERROR("Authentication token mismatch '%s' <> '%s'", authToken.asString().c_str(), oAccount->getAuthToken());
			return new MPC2CResult(kMPC2CResultCode_Unauthorized, "Authentication token mismatch", __pcContentPtr, __nContentSize);
		}
		// delete the SIP account/address
		if(!m_oDb->deleteAccountSipCallerById(id.asInt64()))
		{
			return new MPC2CResult(kMPC2CResultCode_InternalError, "Failed to delete SIP caller account", __pcContentPtr, __nContentSize);
		}
		MPObjectWrapper<MPData*> oData = serializeAccount(kJsonValue_ActionResp_AccountSipCallerDelete, oAccount);
		return new MPC2CResult(kMPC2CResultCode_Success, "SIP address deleted", oData->getDataPtr(), oData->getDataSize());
	}	

	TSK_DEBUG_ERROR("'%s' not valid JSON action", action.asString().c_str());
	return new MPC2CResult(kMPC2CResultCode_InvalidDataType, "Invalid action type");
}

MPObjectWrapper<MPC2CResult*> MPC2CTransport::addAccountAndSendActivationMail(const char* pcEmail, const char* pcName)const
{
	if(!pcEmail || !pcName)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return new MPC2CResult(kMPC2CResultCode_InternalError, "Invalid parameter");
	}

	static const char __pcContentPtr[] = kJsonContent_ActionResp_AccountAdd;
	static const size_t __nContentSize = sizeof(__pcContentPtr) - 1/*\0*/;

	// kill zombibies
	if(MPC2CTransport::s_nAccountsCount == 0 || (MPC2CTransport::s_nAccountsCount % kMaxNewAccountBeforeKillingZombies) == 0)
	{
		m_oDb->killZombieAccounts(kZombieMaxLifeTimeExpectancy);
	}
	++MPC2CTransport::s_nAccountsCount;
	
	// create or update the account
	MPObjectWrapper<MPDbAccount*> oAccount = m_oDb->selectAccountByEmail(pcEmail);
	if(oAccount)
	{
		oAccount->setName(pcName);
		if(m_oDb->updateAccount(oAccount))
		{
			// send activation email
			sendActivationMail(oAccount->getEmail(), oAccount->getName(), oAccount->getActivationCode(), oAccount->getPassword());
			return new MPC2CResult(kMPC2CResultCode_Success, "Account updated", __pcContentPtr, __nContentSize);
		}
		return new MPC2CResult(kMPC2CResultCode_InternalError, "Failed to update account", __pcContentPtr, __nContentSize);
	}
	else
	{
		oAccount = new MPDbAccount(
				MPDB_ACCOUNT_INVALID_ID,
				WEBRTC2SIP_C2C_SOFT_VERSION,
				WEBRTC2SIP_C2C_DATABASE_VERSION,
				pcName,
				pcEmail
			);
		if(m_oDb->addAccount(oAccount))
		{
			// send activation email
			sendActivationMail(oAccount->getEmail(), oAccount->getName(), oAccount->getActivationCode(), oAccount->getPassword());
			return new MPC2CResult(kMPC2CResultCode_Success, "Account added", __pcContentPtr, __nContentSize);
		}
		return new MPC2CResult(kMPC2CResultCode_InternalError, "Failed to add account", __pcContentPtr, __nContentSize);
	}
}

bool MPC2CTransport::sendActivationMail(const char* pcEmail, const char* pcName, const char* pcActivationCode, const char* pcPassword)const
{
	if(!pcEmail || !pcName || !pcPassword || !pcActivationCode)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	if(!m_oMailTransport)
	{
		TSK_DEBUG_ERROR("No mail transport associated to this click-to-call service");
		return false;
	}

	char *pEmailContent = NULL, *pEmail64 = NULL;
	bool bRet = false;

	// base64(email)
	tsk_base64_encode((const uint8_t*)pcEmail, tsk_strlen(pcEmail), &pEmail64);

	int len = tsk_sprintf(&pEmailContent, 
		"Hello %s,\n\n"
		"Thank you for registering to our click-to-call service.\n"
		"Your activation code is '%s', please visit http://click2dial.org/a/%s%%3B%s to activate your account. This code is only valid for 2 days and will be automatically destroyed.\n"
		"Your link address is http://click2dial.org/u/%s, you can start sharing it with your contacts. \n"
		"Your password is '%s' (without the quotes). Please keep it private. You can use it to sign in at http://click2dial.org.\n"
		"Next steps:\n"
		"\t1. Activate your account\n"
		"\t2. Sign In using your email address and password(%s)\n"
		"\t3. Open your account page(http://click2dial.org/account.htm) and tell us what's your SIP address"
		"\nThanks",
		pcName, pcActivationCode, pEmail64, pcActivationCode, pEmail64, pcPassword, pcPassword);
	if(len > 0)
	{
		bRet = m_oMailTransport->sendMail(pcEmail, kMailWelcomeSubject, pEmailContent, len);
	}
	TSK_FREE(pEmailContent);
	TSK_FREE(pEmail64);
	
	return bRet;
}

bool MPC2CTransport::sendResult(MPObjectWrapper<MPNetPeer*> oPeer, MPObjectWrapper<MPC2CResult*> oResult)const
{
	assert(*oResult && *oPeer);

	bool bRet = false;
	void* pResult = NULL;

	int len = tsk_sprintf(
		(char**)&pResult, 
		"HTTP/1.1 %u %s\r\n"
		"Server: webrtc2sip Media Server " WEBRTC2SIP_VERSION_STRING "\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Content-Length: %u\r\n"
		"Content-Type: " kJsonContentType "\r\n"
		"Connection: Close\r\n"
		"\r\n", oResult->getCode(), oResult->getPhrase(), oResult->getDataSize());

	if(len <= 0 || !pResult)
	{
		goto bail;
	}
	if(oResult->getDataPtr() && oResult->getDataSize())
	{
		if(!(pResult = tsk_realloc(pResult, (len + oResult->getDataSize()))))
		{
			goto bail;
		}
		memcpy(&((uint8_t*)pResult)[len], oResult->getDataPtr(), oResult->getDataSize());
		len += oResult->getDataSize();
	}

	// send data
	bRet = const_cast<MPC2CTransport*>(this)->sendData(oPeer->getFd(), pResult, len);
	
bail:
	TSK_FREE(pResult);
	return bRet;
}

//
// MPC2CRecaptchaValidationCallback
//

MPC2CRecaptchaValidationCallback::MPC2CRecaptchaValidationCallback(const MPC2CTransport* pcTransport)
{
	m_pcTransport = pcTransport;
}

MPC2CRecaptchaValidationCallback::~MPC2CRecaptchaValidationCallback()
{
}

bool MPC2CRecaptchaValidationCallback::onValidationEvent(bool bSucess, MPObjectWrapper<MPRecaptcha*> oRecaptcha)
{
	if (bSucess)
	{
		return m_pcTransport->addAccountAndSendActivationMail(oRecaptcha->getAccountEmail(), oRecaptcha->getAccountName());
	}
	else
	{
		TSK_DEBUG_ERROR("reCAPTCHA validation failed: email=%s, name=%s, response=%s, remoteIP=%s", oRecaptcha->getAccountEmail(), oRecaptcha->getAccountName(), oRecaptcha->getResponse(), oRecaptcha->getRemoteIP());
	}
	return true;
}

//
//	MPC2CTransportCallback
//
MPC2CTransportCallback::MPC2CTransportCallback(const MPC2CTransport* pcTransport)
{
	m_pcTransport = pcTransport;
}

MPC2CTransportCallback::~MPC2CTransportCallback()
{

}

bool MPC2CTransportCallback::onData(MPObjectWrapper<MPNetPeer*> oPeer, size_t &nConsumedBytes)
{
	size_t nDataSize;
	const int8_t* pcDataPtr;
	int32_t endOfheaders;
	bool haveAllContent = false;
	thttp_message_t *httpMessage = tsk_null;
	tsk_ragel_state_t ragelState;
	static const tsk_bool_t bExtractContentFalse = tsk_false;
	static const size_t kHttpOptionsResponseSize = tsk_strlen(kHttpOptionsResponse);
	int ret;

	TSK_DEBUG_INFO("click2call service data = %.*s", oPeer->getDataSize(), oPeer->getDataPtr());


	nConsumedBytes = 0;

	// https://developer.mozilla.org/en-US/docs/HTTP/Access_control_CORS


	/* Check if we have all HTTP headers. */
parse_buffer:
	pcDataPtr = ((const int8_t*)oPeer->getDataPtr()) + nConsumedBytes;
	nDataSize = (oPeer->getDataSize() - nConsumedBytes);

	if((endOfheaders = tsk_strindexOf((const char*)pcDataPtr, nDataSize, "\r\n\r\n"/*2CRLF*/)) < 0)
	{
		TSK_DEBUG_INFO("No all HTTP headers in the TCP buffer");
		goto bail;
	}
	
	/* If we are here this mean that we have all HTTP headers.
	*	==> Parse the HTTP message without the content.
	*/
	tsk_ragel_state_init(&ragelState, (const char*)pcDataPtr, endOfheaders + 4/*2CRLF*/);
	if((ret = thttp_message_parse(&ragelState, &httpMessage, bExtractContentFalse)) == 0)
	{
		const thttp_header_Transfer_Encoding_t* transfer_Encoding;

		/* chunked? */
		if((transfer_Encoding = (const thttp_header_Transfer_Encoding_t*)thttp_message_get_header(httpMessage, thttp_htype_Transfer_Encoding)) && tsk_striequals(transfer_Encoding->encoding, "chunked"))
		{
			const char* start = (const char*)(pcDataPtr + (endOfheaders + 4/*2CRLF*/));
			const char* end = (const char*)(pcDataPtr + nDataSize);
			int index;

			TSK_DEBUG_INFO("HTTP CHUNKED transfer");

			while(start < end)
			{
				/* RFC 2616 - 19.4.6 Introduction of Transfer-Encoding */
				// read chunk-size, chunk-extension (if any) and CRLF
				tsk_size_t chunk_size = (tsk_size_t)tsk_atox(start);
				if((index = tsk_strindexOf(start, (end-start), "\r\n")) >=0)
				{
					start += index + 2/*CRLF*/;
				}
				else
				{
					TSK_DEBUG_INFO("Parsing chunked data has failed.");
					break;
				}

				if(chunk_size == 0 && ((start + 2) <= end) && *start == '\r' && *(start+ 1) == '\n')
				{
					int parsed_len = (start - (const char*)(pcDataPtr)) + 2/*CRLF*/;
#if 0
					tsk_buffer_remove(dialog->buf, 0, parsed_len);
#else
					nConsumedBytes += 
#endif
					haveAllContent = true;
					break;
				}
					
				thttp_message_append_content(httpMessage, start, chunk_size);
				start += chunk_size + 2/*CRLF*/;
			}
		}
		else
		{
			tsk_size_t clen = THTTP_MESSAGE_CONTENT_LENGTH(httpMessage); /* MUST have content-length header. */
			if(clen == 0)
			{ /* No content */
				nConsumedBytes += (endOfheaders + 4/*2CRLF*/);/* Remove HTTP headers and CRLF ==> must never happen */
				haveAllContent = true;
			}
			else
			{ /* There is a content */
				if((endOfheaders + 4/*2CRLF*/ + clen) > nDataSize)
				{ /* There is content but not all the content. */
					TSK_DEBUG_INFO("No all HTTP content in the TCP buffer.");
					goto bail;
				}
				else
				{
					/* Add the content to the message. */
					thttp_message_add_content(httpMessage, tsk_null, pcDataPtr + endOfheaders + 4/*2CRLF*/, clen);
					/* Remove HTTP headers, CRLF and the content. */
					nConsumedBytes += (endOfheaders + 4/*2CRLF*/ + clen);
					haveAllContent = true;
				}
			}
		}
	}
	else
	{
		// fails to parse an HTTP message with all headers
		nConsumedBytes += endOfheaders + 4/*2CRLF*/;
	}
	
	
	if(httpMessage && haveAllContent)
	{
		/* Analyze HTTP message */
		if(THTTP_MESSAGE_IS_REQUEST(httpMessage))
		{
			/* OPTIONS */
			if(tsk_striequals(httpMessage->line.request.method, kHttpMethodOptions))
			{
				// https://developer.mozilla.org/en-US/docs/HTTP/Access_control_CORS
				if(!const_cast<MPC2CTransport*>(m_pcTransport)->sendData(oPeer->getFd(), kHttpOptionsResponse, kHttpOptionsResponseSize))
				{
					TSK_DEBUG_ERROR("Failed to send response to HTTP OPTIONS");
				}
			}
			/* POST */
			else if(tsk_striequals(httpMessage->line.request.method, kHttpMethodPost))
			{
				if(!isJsonContentType(httpMessage))
				{
					m_pcTransport->sendResult(oPeer, new MPC2CResult(kMPC2CResultCode_InvalidData, "Invalid content-type"));
				}
				else if(!THTTP_MESSAGE_HAS_CONTENT(httpMessage))
				{
					m_pcTransport->sendResult(oPeer, new MPC2CResult(kMPC2CResultCode_InvalidData, "Bodiless POST request not allowed"));
				}
				else
				{
					MPObjectWrapper<MPC2CResult*> oResult = m_pcTransport->handleJsonContent(oPeer->getFd(), THTTP_MESSAGE_CONTENT(httpMessage), THTTP_MESSAGE_CONTENT_LENGTH(httpMessage));
					m_pcTransport->sendResult(oPeer, oResult);
				}
			}
		}

		/* Parse next chunck */
		if((nDataSize - nConsumedBytes) >= kStreamChunckMinSize)
		{
			TSK_OBJECT_SAFE_FREE(httpMessage);
			goto parse_buffer;
		}
	}

bail:
	TSK_OBJECT_SAFE_FREE(httpMessage);
	
	return true;
}

bool MPC2CTransportCallback::onConnectionStateChanged(MPObjectWrapper<MPNetPeer*> oPeer)
{
	return true;
}

static const char* getHttpContentType(const thttp_message_t *pcHttpMessage)
{
	const thttp_header_Content_Type_t* contentType;

	if(!pcHttpMessage)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return NULL;
	}

	if((contentType = (const thttp_header_Content_Type_t*)thttp_message_get_header(pcHttpMessage, thttp_htype_Content_Type)))
	{
		return contentType->type;
	}
	return NULL;
}

static bool isJsonContentType(const thttp_message_t *pcHttpMessage)
{
	// content-type without parameters
	const char* pcContentType = getHttpContentType(pcHttpMessage);
	if(pcContentType)
	{
		return tsk_striequals(kJsonContentType, pcContentType);
	}
	return false;
}

}//namespace