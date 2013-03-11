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
#include "mp_mail.h"

#include "tsk_string.h"
#include "tsk_memory.h"
#include "tsk_base64.h"

#include <assert.h>

namespace webrtc2sip {

class MPMailTransportCallback;


//
//	MPMail
//

MPMail::MPMail(const char* pcSrcMailAddr, const char* pcDstMailAddr, const char* pcSubject, const void* pcDataPtr, size_t nDataSize)
{
	m_pSrcMailAddr = tsk_strdup(pcSrcMailAddr);
	m_pDstMailAddr = tsk_strdup(pcDstMailAddr);
	m_pSubject = tsk_strdup(pcSubject);
	if(nDataSize && pcDataPtr && (m_pDataPtr = tsk_malloc(nDataSize)))
	{
		memcpy(m_pDataPtr, pcDataPtr, nDataSize);
		m_nDataSize = nDataSize;
	}
}

MPMail::~MPMail()
{
	TSK_FREE(m_pSrcMailAddr);
	TSK_FREE(m_pDstMailAddr);
	TSK_FREE(m_pSubject);
	TSK_FREE(m_pDataPtr);
}


//
//	MPMailTransport
//

MPMailTransport::MPMailTransport(bool isSecure, const char* pcLocalIP, unsigned short nLocalPort, const char* pcSmtpHost, unsigned short nSmtpPort, const char* pcEmail, const char* pcAuthName, const char* pcAuthPwd)
: MPNetTransport(isSecure ? MPNetTransporType_TLS : MPNetTransporType_TCP, pcLocalIP, nLocalPort)
, m_nConnectedFd(kMPNetFdInvalid)
, m_bConnectToPending(false)
, m_bNeedToAuthenticate(true)
, m_pAuthName64(NULL)
, m_pAuthPwd64(NULL)
, m_pTempBuffPtr(NULL)
, m_nTempBuffSize(0)
{
	m_oCallback = new MPMailTransportCallback(this);
	setCallback(*m_oCallback);

	m_pSmtpHost = tsk_strdup(pcSmtpHost);
	m_nSmtpPort = nSmtpPort;
	m_pEmail = tsk_strdup(pcEmail);
	assert(tsk_base64_encode((const uint8_t*)pcAuthName, tsk_strlen(pcAuthName), &m_pAuthName64));
	assert(tsk_base64_encode((const uint8_t*)pcAuthPwd, tsk_strlen(pcAuthPwd), &m_pAuthPwd64));
}

MPMailTransport::~MPMailTransport()
{
	setCallback(NULL);
	
	m_oPendingMails.clear();

	TSK_FREE(m_pSmtpHost);
	TSK_FREE(m_pEmail);
	TSK_FREE(m_pAuthName64);
	TSK_FREE(m_pAuthPwd64);
	TSK_FREE(m_pTempBuffPtr);
}

bool MPMailTransport::sendMail(const char* pcDstMailAddr, const char* pcSubject, const void* pcDataPr, size_t nDataSize, const char* pcSrcMailAddr /*= NULL*/)
{
	if(!pcDstMailAddr || !pcDataPr || !nDataSize)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	if(!pcSrcMailAddr)
	{
		pcSrcMailAddr = m_pEmail;
	}

	// create Mail object
	MPObjectWrapper<MPMail*> oMail = new MPMail(pcSrcMailAddr, pcDstMailAddr, pcSubject, pcDataPr, nDataSize);

	if(!MPNetFd_IsValid(m_nConnectedFd) || !isConnected(m_nConnectedFd))
	{
		TSK_DEBUG_INFO("Mail transport not connected");
		// save the mail until we get connected
		m_oPendingMails.push_back(oMail);

		if(!m_bConnectToPending)
		{
			m_bConnectToPending = true;
			TSK_DEBUG_INFO("Connect Mail transport to: [%s:%d]", m_pSmtpHost, m_nSmtpPort);
			return MPNetFd_IsValid(connectTo(m_pSmtpHost, m_nSmtpPort));
		}
		return true;
	}
	
	// if the code reach this line means we are connected
	return sendMail(oMail);
}

bool MPMailTransport::sendMail(MPObjectWrapper<MPMail*> oMail)
{
	assert(oMail);

	if(!MPNetFd_IsValid(m_nConnectedFd) || !isConnected(m_nConnectedFd))
	{
		TSK_DEBUG_ERROR("Mail service not connected");
		return false;
	}

	size_t nSmtpHostLen = tsk_strlen(m_pSmtpHost);
	size_t nAuthName64Len = tsk_strlen(m_pAuthName64);
	size_t nAuthPwd64Len = tsk_strlen(m_pAuthPwd64);
	size_t nSrcMailAddrLen = tsk_strlen(oMail->getSrcMailAddr());
	size_t nDstMailAddrLen = tsk_strlen(oMail->getDstMailAddr());
	size_t nSubjectLen = tsk_strlen(oMail->getSubject());

	size_t nContentSize 
		= 5 /*EHLO */ + nSmtpHostLen + 2 /*\r\n*/
		+ (!m_bNeedToAuthenticate ? 0 : (10 /*AUTH LOGIN*/ + 2 /*\r\n*/))
		+ (!m_bNeedToAuthenticate ? 0 : (nAuthName64Len + 2 /*\r\n*/))
		+ (!m_bNeedToAuthenticate ? 0 : (nAuthPwd64Len + 2 /*\r\n*/))
		+ 11 /*MAIL FROM:<*/ + nSrcMailAddrLen + 1 /*>*/ + 2 /*\r\n*/
		+ 9 /*RCPT TO:<*/ + nDstMailAddrLen + 1 /*>*/ + 2 /*\r\n*/
		+ 4 /*DATA*/ + 2 /*\r\n*/
		+ 8 /*SUBJECT:*/ + nSubjectLen + 2 /*\r\n*/
		+ 5 /*From:*/ + nSrcMailAddrLen + 2 /*\r\n*/
		+ 3 /*To:*/ + nDstMailAddrLen + 2 /*\r\n*/
		+ 2 /*\r\n*/
		+ oMail->getDataSize() + 2 /*\r\n*/
		+ 5 /*\r\n.\r\n*/;

	if(m_nTempBuffSize < nContentSize)
	{
		if(!(m_pTempBuffPtr = (uint8_t*)tsk_realloc(m_pTempBuffPtr, nContentSize)))
		{
			m_nTempBuffSize = 0;
			return false;
		}
		m_nTempBuffSize = nContentSize;
	}

	size_t nIndex = 0;

	memcpy(&m_pTempBuffPtr[nIndex], "EHLO ", 5); nIndex += 5;
	memcpy(&m_pTempBuffPtr[nIndex], m_pSmtpHost, nSmtpHostLen); nIndex += nSmtpHostLen;
	memcpy(&m_pTempBuffPtr[nIndex], "\r\n", 2); nIndex += 2;

	if(m_bNeedToAuthenticate){
		memcpy(&m_pTempBuffPtr[nIndex], "AUTH LOGIN\r\n", 12); nIndex += 12;

		memcpy(&m_pTempBuffPtr[nIndex], m_pAuthName64, nAuthName64Len); nIndex += nAuthName64Len;
		memcpy(&m_pTempBuffPtr[nIndex], "\r\n", 2); nIndex += 2;

		memcpy(&m_pTempBuffPtr[nIndex], m_pAuthPwd64, nAuthPwd64Len); nIndex += nAuthPwd64Len;
		memcpy(&m_pTempBuffPtr[nIndex], "\r\n", 2); nIndex += 2;
	}

	memcpy(&m_pTempBuffPtr[nIndex], "MAIL FROM:<", 11); nIndex += 11;
	memcpy(&m_pTempBuffPtr[nIndex], oMail->getSrcMailAddr(), nSrcMailAddrLen); nIndex += nSrcMailAddrLen;
	memcpy(&m_pTempBuffPtr[nIndex], ">\r\n", 3); nIndex += 3;

	memcpy(&m_pTempBuffPtr[nIndex], "RCPT TO:<", 9); nIndex += 9;
	memcpy(&m_pTempBuffPtr[nIndex], oMail->getDstMailAddr(), nDstMailAddrLen); nIndex += nDstMailAddrLen;
	memcpy(&m_pTempBuffPtr[nIndex], ">\r\n", 3); nIndex += 3;

	memcpy(&m_pTempBuffPtr[nIndex], "DATA\r\n", 6); nIndex += 6;

	memcpy(&m_pTempBuffPtr[nIndex], "SUBJECT:", 8); nIndex += 8;
	memcpy(&m_pTempBuffPtr[nIndex], oMail->getSubject(), nSubjectLen); nIndex += nSubjectLen;
	memcpy(&m_pTempBuffPtr[nIndex], "\r\n", 2); nIndex += 2;

	memcpy(&m_pTempBuffPtr[nIndex], "From:", 5); nIndex += 5;
	memcpy(&m_pTempBuffPtr[nIndex], oMail->getSrcMailAddr(), nSrcMailAddrLen); nIndex += nSrcMailAddrLen;
	memcpy(&m_pTempBuffPtr[nIndex], "\r\n", 2); nIndex += 2;

	memcpy(&m_pTempBuffPtr[nIndex], "To:", 3); nIndex += 3;
	memcpy(&m_pTempBuffPtr[nIndex], oMail->getDstMailAddr(), nDstMailAddrLen); nIndex += nDstMailAddrLen;
	memcpy(&m_pTempBuffPtr[nIndex], "\r\n", 2); nIndex += 2;

	memcpy(&m_pTempBuffPtr[nIndex], "\r\n", 2); nIndex += 2;

	memcpy(&m_pTempBuffPtr[nIndex], oMail->getDataPtr(), oMail->getDataSize()); nIndex += oMail->getDataSize();
	memcpy(&m_pTempBuffPtr[nIndex], "\r\n", 2); nIndex += 2;

	memcpy(&m_pTempBuffPtr[nIndex], "\r\n.\r\n", 5); nIndex += 5;

	TSK_DEBUG_INFO("Send Mail = %.*s", nIndex, m_pTempBuffPtr);

	if(sendData(m_nConnectedFd, m_pTempBuffPtr, nIndex))
	{
		TSK_DEBUG_INFO("Mail sent");
		return true;
	}
	else
	{
		TSK_DEBUG_ERROR("Failed to send mail to : %s", oMail->getDstMailAddr());
		return false;
	}
}

bool MPMailTransport::sendPendingMails()
{
	while(MPNetFd_IsValid(m_nConnectedFd) && isConnected(m_nConnectedFd) && !m_oPendingMails.empty())
	{
		MPObjectWrapper<MPMail*> oMail = *m_oPendingMails.begin();
		m_oPendingMails.pop_front();
		if(!sendMail(oMail))
		{
			TSK_DEBUG_ERROR("Failed to send mail to : %s", oMail->getDstMailAddr());
			return false;
		}
		m_bNeedToAuthenticate = false; // subsequent messages should not be authenticated unless new connection is used
	}

	return true;
}


//
//	MPMailTransportCallback
//
MPMailTransportCallback::MPMailTransportCallback(const MPMailTransport* pcTransport)
{
	m_pcTransport = pcTransport;
}

MPMailTransportCallback::~MPMailTransportCallback()
{

}

bool MPMailTransportCallback::onData(MPObjectWrapper<MPNetPeer*> oPeer, size_t &nConsumedBytes)
{
	TSK_DEBUG_INFO("mail service data = %.*s", oPeer->getDataSize(), oPeer->getDataPtr());

	return true;
}

bool MPMailTransportCallback::onConnectionStateChanged(MPObjectWrapper<MPNetPeer*> oPeer)
{
	const_cast<MPMailTransport*>(m_pcTransport)->m_bConnectToPending = false;
	const_cast<MPMailTransport*>(m_pcTransport)->m_nConnectedFd = oPeer->isConnected() ? oPeer->getFd() : kMPNetFdInvalid;

	if(oPeer->isConnected())
	{
		return const_cast<MPMailTransport*>(m_pcTransport)->sendPendingMails();
	}
	else{
		// next time we need to authenticate
		const_cast<MPMailTransport*>(m_pcTransport)->m_bNeedToAuthenticate = true;
	}
	return true;
}


}//namespace