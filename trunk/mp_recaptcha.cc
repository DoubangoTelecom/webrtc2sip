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
#include "mp_recaptcha.h"
#include "jsoncpp/json/json.h"

#include "tsk_string.h"
#include "tsk_memory.h"
#include "tsk_base64.h"

#include "tinyhttp.h"
#include "tinyhttp/parsers/thttp_parser_url.h"

#include <assert.h>

// http://www.codedodle.com/2014/12/google-new-recaptcha-using-javascript.html

namespace webrtc2sip {

class MPRecaptchaTransportCallback;

//
//	MPRecaptcha
//

MPRecaptcha::MPRecaptcha(MPNetFd nFd, const char* pcAccountName, const char* pcAccountEmail, const char* pcResponse, const char* pcRemoteIP /*= NULL*/)
: m_nFd(nFd)
, m_bConnected(false)
{
	assert(MPNetFd_IsValid(m_nFd));
	m_pAccountName = tsk_strdup(pcAccountName); assert(m_pAccountName);
	m_pAccountEmail = tsk_strdup(pcAccountEmail); assert(m_pAccountEmail);
	m_pResponse = tsk_strdup(pcResponse); assert(m_pResponse);
	m_pRemoteIP = tsk_strdup(pcRemoteIP);
	
}

MPRecaptcha::~MPRecaptcha()
{
	TSK_FREE(m_pAccountName);
	TSK_FREE(m_pAccountEmail);
	TSK_FREE(m_pResponse);
	TSK_FREE(m_pRemoteIP);
}

void MPRecaptcha::setRemoteIP(const char* pcRemoteIP)
{
	tsk_strupdate(&m_pRemoteIP, pcRemoteIP);
}


//
//	MPRecaptchaTransport
//

MPRecaptchaTransport::MPRecaptchaTransport(const char* pcVerifySiteUrl, const char* pcSecret)
: MPNetTransport(tsk_strindexOf(pcVerifySiteUrl, tsk_strlen(pcVerifySiteUrl), "https:") == 0 ? MPNetTransporType_TLS : MPNetTransporType_TCP, TNET_SOCKET_HOST_ANY, TNET_SOCKET_PORT_ANY)
, m_pWrappedHttpUrl(NULL)
, m_pWrappedRecaptchasMutex(NULL)
{
	m_pWrappedRecaptchasMutex = tsk_mutex_create(); assert(m_pWrappedRecaptchasMutex);
	m_pWrappedHttpUrl = thttp_url_parse(pcVerifySiteUrl, tsk_strlen(pcVerifySiteUrl));
	assert(m_pWrappedHttpUrl && !tsk_strnullORempty(m_pWrappedHttpUrl->host));

	m_pVerifySiteUrl = tsk_strdup(pcVerifySiteUrl); assert(m_pVerifySiteUrl);
	m_pSecret = tsk_strdup(pcSecret); assert(m_pSecret);
	m_oCallback = new MPRecaptchaTransportCallback(this);
	setCallback(*m_oCallback);
}

MPRecaptchaTransport::~MPRecaptchaTransport()
{
	setCallback(NULL);
	
	m_Recaptchas.clear();

	TSK_FREE(m_pVerifySiteUrl);
	TSK_FREE(m_pSecret);
	TSK_OBJECT_SAFE_FREE(m_pWrappedHttpUrl);
	if (m_pWrappedRecaptchasMutex)
	{
		tsk_mutex_destroy(&m_pWrappedRecaptchasMutex);
	}
}

bool MPRecaptchaTransport::validate(MPNetFd nLocalFd, const char* pcAccountName, const char* pcAccountEmail, const char* pcResponse)
{
	if (tsk_strnullORempty(pcAccountName) || tsk_strnullORempty(pcAccountEmail))
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	tnet_ip_t remote_ip;
	bool bGotRemoteIP = false;
	if (tnet_get_peerip(nLocalFd, &remote_ip) != 0)
	{
		remote_ip[0] = '\0';
	}
	
	MPNetFd nFd = connectTo(m_pWrappedHttpUrl->host, m_pWrappedHttpUrl->port);
	if (!MPNetFd_IsValid(nFd))
	{
		TSK_DEBUG_ERROR("Failed to connect to %s:%d", m_pWrappedHttpUrl->host, m_pWrappedHttpUrl->port);
		return false;
	}

	MPObjectWrapper<MPRecaptcha*> oRecaptcha = new MPRecaptcha(nFd, pcAccountName, pcAccountEmail, pcResponse, remote_ip);
	assert(*oRecaptcha);
	insertRecaptcha(oRecaptcha);
	if (isConnected(nFd))
	{
		return sendRecaptcha(oRecaptcha);
	}
	return true;
}

void MPRecaptchaTransport::insertRecaptcha(MPObjectWrapper<MPRecaptcha*> oRecaptcha)
{
	tsk_mutex_lock(m_pWrappedRecaptchasMutex);
	if (oRecaptcha)
	{
		m_Recaptchas.insert( std::pair<MPNetFd, MPObjectWrapper<MPRecaptcha*> >(oRecaptcha->getFd(), oRecaptcha) );
	}
	tsk_mutex_unlock(m_pWrappedRecaptchasMutex);
}

void MPRecaptchaTransport::removeRecaptcha(MPNetFd nFd)
{
	tsk_mutex_lock(m_pWrappedRecaptchasMutex);
	std::map<MPNetFd, MPObjectWrapper<MPRecaptcha*> >::iterator iter;
	if ((iter = m_Recaptchas.find(nFd)) != m_Recaptchas.end())
	{
		MPObjectWrapper<MPRecaptcha*> oRecaptcha = iter->second;
		m_Recaptchas.erase(iter);
	}
	tsk_mutex_unlock(m_pWrappedRecaptchasMutex);
}

MPObjectWrapper<MPRecaptcha*> MPRecaptchaTransport::getRecaptchaFd(MPNetFd nFd)
{
	MPObjectWrapper<MPRecaptcha*> m_Recaptcha = NULL;

	tsk_mutex_lock(m_pWrappedRecaptchasMutex);

	std::map<MPNetFd, MPObjectWrapper<MPRecaptcha*> >::iterator iter = m_Recaptchas.find(nFd);
	if(iter != m_Recaptchas.end()){
		m_Recaptcha = iter->second;
	}
	tsk_mutex_unlock(m_pWrappedRecaptchasMutex);

	return m_Recaptcha;
}

bool MPRecaptchaTransport::sendRecaptcha(MPObjectWrapper<MPRecaptcha*> oRecaptcha)
{
	MPObjectWrapper<MPNetPeer*> oPeer = getPeerByFd(oRecaptcha->getFd());
	if (oPeer)
	{
		// Use "1.0" protocol to avoid chuncked transfer: https://en.wikipedia.org/wiki/Chunked_transfer_encoding
#define GET_MSG \
	"GET %s HTTP/1.0\r\n" \
	"Host: %s:%d\r\n" \
	"Pragma: No-Cache\r\n" \
	"User-Agent: webrtc2sip Media Server " WEBRTC2SIP_VERSION_STRING "\r\n" \
	"Content-Length: 0\r\n" \
	"Connection: Close\r\n" \
	"Accept: application/json; charset=utf-8\r\n" \
	"\r\n"

		char* httpMsg = tsk_null;
		char* hpath = tsk_null;
#if 1
		tsk_sprintf(&hpath, "/%s?secret=%s&response=%s",
			m_pWrappedHttpUrl->hpath, m_pSecret, oRecaptcha->getResponse());
#else
		tsk_sprintf(&hpath, "%s?secret=%s&response=%s",
			m_pVerifySiteUrl, m_pSecret, oRecaptcha->getResponse());
#endif
		if (!tsk_strnullORempty(oRecaptcha->getRemoteIP()))
		{
			tsk_strcat_2(&hpath, "&remoteip=%s", oRecaptcha->getRemoteIP());
		}

		int httpLen = tsk_sprintf(&httpMsg, GET_MSG,
			hpath,
			m_pWrappedHttpUrl->host, m_pWrappedHttpUrl->port);
		TSK_FREE(hpath);
		bool bRet = sendData(oPeer->getFd(), httpMsg, httpLen);
		TSK_FREE(httpMsg);
		return bRet;
	}
	return false;
}

//
//	MPRecaptchaTransportCallback
//
MPRecaptchaTransportCallback::MPRecaptchaTransportCallback(const MPRecaptchaTransport* pcTransport)
{
	m_pcTransport = pcTransport;
}

MPRecaptchaTransportCallback::~MPRecaptchaTransportCallback()
{

}

bool MPRecaptchaTransportCallback::onData(MPObjectWrapper<MPNetPeer*> oPeer, size_t &nConsumedBytes)
{
	TSK_DEBUG_INFO("Recaptcha(fd=%d) service data = %.*s", oPeer->getFd(), oPeer->getDataSize(), oPeer->getDataPtr());

	int endOfheaders;
	MPObjectWrapper<MPRecaptcha*> oRecaptcha;

	oRecaptcha = const_cast<MPRecaptchaTransport*>(m_pcTransport)->getRecaptchaFd(oPeer->getFd());

	if (!oRecaptcha)
	{
		TSK_DEBUG_ERROR("Failed to find captcha with fd = %d", oPeer->getFd());
		return false;
	}

	nConsumedBytes = 0;

	if ((endOfheaders = tsk_strindexOf((const char*)oPeer->getDataPtr(), oPeer->getDataSize(), "\r\n\r\n"/*2CRLF*/)) < 0)
	{
		TSK_DEBUG_INFO("Recaptcha: No all HTTP headers in the TCP buffer");
	}
	else
	{
		thttp_message_t *httpMessage = tsk_null;
		tsk_ragel_state_t ragelState;
		static const tsk_bool_t bExtractContentFalse = tsk_false;

		tsk_ragel_state_init(&ragelState, (const char*)oPeer->getDataPtr(), endOfheaders + 4/*2CRLF*/);
		if (thttp_message_parse(&ragelState, &httpMessage, bExtractContentFalse) == 0)
		{
			tsk_size_t clen = 0;
			if (httpMessage->Content_Length) {
				THTTP_MESSAGE_CONTENT_LENGTH(httpMessage);
			}
			else {
				// Content-Length missing: bug on Google side
				clen = oPeer->getDataSize() - (endOfheaders + 4/*2CRLF*/);
			}

			if (clen == 0)
			{ /* No content */
				nConsumedBytes += (endOfheaders + 4/*2CRLF*/);/* Remove HTTP headers and CRLF ==> must never happen */
			}
			else
			{ /* There is a content */
				if ((endOfheaders + 4/*2CRLF*/ + clen) > oPeer->getDataSize())
				{ /* There is content but not all the content. */
					TSK_DEBUG_INFO("Recaptcha: No all HTTP headers in the TCP buffer");
				}
				else
				{
					/* Remove HTTP headers, CRLF and the content. */
					nConsumedBytes += (endOfheaders + 4/*2CRLF*/ + clen);

					const char* pcContentPtr = (const char*)(((const char*)oPeer->getDataPtr()) + endOfheaders + 4/*2CRLF*/);
					Json::Value root;
					Json::Reader reader;
					bool parsingSuccessful = reader.parse(pcContentPtr, (pcContentPtr + clen), root);
					if (!parsingSuccessful)
					{
						TSK_DEBUG_ERROR("Failed to parse JSON content: %.*s", (int)clen, pcContentPtr);
						if (m_pcTransport->m_oValidationCallback)
						{
							m_pcTransport->m_oValidationCallback->onValidationEvent(false, oRecaptcha);
						}
					}
					else
					{
						bool bSuccess = (root["success"].isBool() && root["success"].asBool());
						if (m_pcTransport->m_oValidationCallback)
						{
							m_pcTransport->m_oValidationCallback->onValidationEvent(bSuccess, oRecaptcha);
						}
					}
				}
			}
		}
		TSK_OBJECT_SAFE_FREE(httpMessage);
	}

	return true;
}

bool MPRecaptchaTransportCallback::onConnectionStateChanged(MPObjectWrapper<MPNetPeer*> oPeer)
{
	if (oPeer->isConnected())
	{
		MPObjectWrapper<MPRecaptcha*> oRecaptcha = const_cast<MPRecaptchaTransport*>(m_pcTransport)->getRecaptchaFd(oPeer->getFd());
		if (oRecaptcha)
		{
			oRecaptcha->setConnected(true);
			return const_cast<MPRecaptchaTransport*>(m_pcTransport)->sendRecaptcha(oRecaptcha);
		}
	}
	else
	{
		if (!const_cast<MPRecaptchaTransport*>(m_pcTransport)->havePeer(oPeer)) // removed?
		{
			const_cast<MPRecaptchaTransport*>(m_pcTransport)->removeRecaptcha(oPeer->getFd());
		}
	}
	return true;
}


}//namespace