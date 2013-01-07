/* Copyright (C) 2012 Doubango Telecom <http://www.doubango.org>
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
#include "mp_engine.h"
#include "mp_proxyplugin_mgr.h"

#include "SipStack.h"
#include "MediaSessionMgr.h"

#include <assert.h>
#include <functional>
#include <algorithm>

#if !defined(MP_USER_AGENT_STR)
#	define	MP_USER_AGENT_STR	"webrtc2sip Media Server 2.0"
#endif

#if !defined(MP_REALM)
#	define	MP_REALM	"webrtc2sip.org"
#endif
#if !defined(MP_IMPI)
#	define	MP_IMPI	"webrtc2sip"
#endif
#if !defined(MP_IMPU)
#	define	MP_IMPU	"webrtc2sip"
#endif

namespace webrtc2sip {

bool MPEngine::g_bInitialized = false;

struct _PeerBySessionIdLeft: public std::binary_function< std::pair<uint64_t, MPObjectWrapper<MPPeer*> >, uint64_t, bool > {
  bool operator () ( std::pair<uint64_t, MPObjectWrapper<MPPeer*> > pair, const uint64_t &nSessionId ) const {
	  return pair.second->getSessionIdLeft() == nSessionId;
    }
};
struct _PeerBySessionIdRight: public std::binary_function< std::pair<uint64_t, MPObjectWrapper<MPPeer*> >, uint64_t, bool > {
  bool operator () ( std::pair<uint64_t, MPObjectWrapper<MPPeer*> > pair, const uint64_t &nSessionId ) const {
	  return pair.second->getSessionIdRight() == nSessionId;
    }
};

MPEngine::MPEngine(const char* pRealmUri, const char* pPrivateIdentity, const char* pPublicIdentity)
: m_bStarted(false)
, m_bValid(false)
{
	if(!MPEngine::g_bInitialized){
		SipStack::initialize();
		MPProxyPluginMgr::initialize();
		MPEngine::g_bInitialized = true;
	}

	m_oMutex = MPMutex::New();
	m_oMutexPeers = MPMutex::New();

	if((m_oCallback = MPSipCallback::New(this))){
		if((m_oStack = MPSipStack::New(m_oCallback, pRealmUri, pPrivateIdentity, pPublicIdentity))){
			if(!const_cast<SipStack*>(m_oStack->getWrappedStack())->setMode(tsip_stack_mode_webrtc2sip)){
				TSK_DEBUG_ERROR("SipStack::setModeServer failed");
				return;
			}
			const_cast<SipStack*>(m_oStack->getWrappedStack())->addHeader("User-Agent", MP_USER_AGENT_STR);
			m_bValid = true;
		}
	}

}

MPEngine::~MPEngine()
{
	
}

bool MPEngine::setDebugLevel(const char* pcLevel)
{
	struct debug_level { const char* name; int level; };
	static const debug_level debug_levels[] =
	{
		{"INFO", DEBUG_LEVEL_INFO},
		{"WARN", DEBUG_LEVEL_WARN},
		{"ERROR", DEBUG_LEVEL_ERROR},
		{"FATAL", DEBUG_LEVEL_FATAL},
	};
	static const int debug_levels_count = sizeof(debug_levels)/sizeof(debug_levels[0]);
	int i;
	for(i = 0; i < debug_levels_count; ++i){
		if(tsk_striequals(debug_levels[i].name, pcLevel)){
			tsk_debug_set_level(debug_levels[i].level);
			return true;
		}
	}
	return false;
}

bool MPEngine::addTransport(const char* pTransport, uint16_t pLocalPort, const char* pLocalIP /*= tsk_null*/)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}
	bool bRet = const_cast<SipStack*>(m_oStack->getWrappedStack())->setLocalIP(pLocalIP, pTransport);
	bRet &= const_cast<SipStack*>(m_oStack->getWrappedStack())->setLocalPort(pLocalPort, pTransport);
	return bRet;
}

bool MPEngine::setRtpSymetricEnabled(bool bEnabled)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}
	return MediaSessionMgr::defaultsSetRtpSymetricEnabled(bEnabled);
}

bool MPEngine::set100relEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSet100relEnabled(bEnabled);
}

bool MPEngine::setMediaCoderEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSetByPassEncoding(!bEnabled) &&
		MediaSessionMgr::defaultsSetByPassDecoding(!bEnabled);
}

bool MPEngine::setVideoJbEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSetVideoJbEnabled(false);
}

bool MPEngine::setRtpBuffSize(int32_t nSize)
{
	return MediaSessionMgr::defaultsSetRtpBuffSize(0xffff);
}

bool MPEngine::setAvpfTail(int32_t nMin, int32_t nMax)
{
	if(!nMin || (nMin > nMax))
	{
		TSK_DEBUG_ERROR("[%d-%d] not valid as AVPF tail", nMin, nMax);
		return false;
	}
	return MediaSessionMgr::defaultsSetAvpfTail(nMin, nMax);
}

bool MPEngine::setPrefVideoSize(const char* pcPrefVideoSize)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}

	int i;
	struct pref_video_size { const char* name; tmedia_pref_video_size_t size; };
	static const pref_video_size pref_video_sizes[] =
	{
		{"sqcif", tmedia_pref_video_size_sqcif}, // 128 x 98
		{"qcif", tmedia_pref_video_size_qcif}, // 176 x 144
		{"qvga", tmedia_pref_video_size_qvga}, // 320 x 240
		{"cif", tmedia_pref_video_size_cif}, // 352 x 288
		{"hvga", tmedia_pref_video_size_hvga}, // 480 x 320
		{"vga", tmedia_pref_video_size_vga}, // 640 x 480
		{"4cif", tmedia_pref_video_size_4cif}, // 704 x 576
		{"svga", tmedia_pref_video_size_svga}, // 800 x 600
		{"480p", tmedia_pref_video_size_480p}, // 852 x 480
		{"720p", tmedia_pref_video_size_720p}, // 1280 x 720
		{"16cif", tmedia_pref_video_size_16cif}, // 1408 x 1152
		{"1080p", tmedia_pref_video_size_1080p}, // 1920 x 1080
	};
	static const int pref_video_sizes_count = sizeof(pref_video_sizes)/sizeof(pref_video_sizes[0]);
	
	for(i = 0; i < pref_video_sizes_count; ++i){
		if(tsk_striequals(pref_video_sizes[i].name, pcPrefVideoSize)){
			return MediaSessionMgr::defaultsSetPrefVideoSize(pref_video_sizes[i].size);
		}
	}
	TSK_DEBUG_ERROR("%s not valid as video size. Valid values", pcPrefVideoSize);
	return false;
}

bool MPEngine::setSSLCertificate(const char* pcPrivateKey, const char* pcPublicKey, const char* pcCA, bool bVerify /*= false*/)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}
	return const_cast<SipStack*>(m_oStack->getWrappedStack())->setSSLCretificates(
				pcPrivateKey,
				pcPublicKey,
				pcCA,
				bVerify
			);
}

bool MPEngine::setCodecs(int64_t nCodecs)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}
	const_cast<SipStack*>(m_oStack->getWrappedStack())->setCodecs_2(nCodecs);
	return true;
}

bool MPEngine::setSRTPMode(const char* pcMode)
{
	if(!pcMode){
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	if(tsk_striequals(pcMode, "none")){
		return MediaSessionMgr::defaultsSetSRtpMode(tmedia_srtp_mode_none);
	}
	else if(tsk_striequals(pcMode, "optional")){
		return MediaSessionMgr::defaultsSetSRtpMode(tmedia_srtp_mode_optional);
	}
	else if(tsk_striequals(pcMode, "mandatory")){
		return MediaSessionMgr::defaultsSetSRtpMode(tmedia_srtp_mode_mandatory);
	}
	else{
		TSK_DEBUG_ERROR("%s note valid as SRTP mode", pcMode);
		return false;
	}
}

bool MPEngine::setSRTPType(const char* pcTypesCommaSep)
{
	if(!pcTypesCommaSep){
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	tmedia_srtp_type_t srtp_type = tmedia_srtp_type_none;

	if(tsk_strcontains(pcTypesCommaSep, tsk_strlen(pcTypesCommaSep), "sdes"))
	{
		srtp_type = (tmedia_srtp_type_t)(srtp_type | tmedia_srtp_type_sdes);
	}
	if(tsk_strcontains(pcTypesCommaSep, tsk_strlen(pcTypesCommaSep), "dtls"))
	{
		srtp_type = (tmedia_srtp_type_t)(srtp_type | tmedia_srtp_type_dtls);
	}

	return MediaSessionMgr::defaultsSetSRtpType(srtp_type);
}

bool MPEngine::addDNSServer(const char* pcDNSServer)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}
	return const_cast<SipStack*>(m_oStack->getWrappedStack())->addDnsServer(pcDNSServer);
}

bool MPEngine::start()
{
	int ret = 0;
	m_oMutex->lock();

	if(isStarted()){
		goto bail;
	}

	if(!isValid()){
		TSK_DEBUG_ERROR("Engine not valid");
		ret = -1;
		goto bail;
	}
	if(const_cast<SipStack*>(m_oStack->getWrappedStack())->start()){
		setStarted(true);
	}
	else{
		TSK_DEBUG_ERROR("Failed to start SIP stack");
		ret = -2;
		goto bail;
	}

bail:
	m_oMutex->unlock();
	return (ret == 0);
}

bool MPEngine::stop()
{
	int ret = 0;

	m_oMutex->lock();

	if(!isStarted()){
		goto bail;
	}

	if(!isValid()){
		TSK_DEBUG_ERROR("Engine not valid");
		ret = -1;
		goto bail;
	}

	if(const_cast<SipStack*>(m_oStack->getWrappedStack())->stop()){
		setStarted(false);
	}
	else{
		TSK_DEBUG_ERROR("Failed to stop SIP stack");
		ret = -2;
		goto bail;
	}

bail:
	m_oMutex->unlock();
	return (ret == 0);
}

MPObjectWrapper<MPPeer*> MPEngine::getPeerById(uint64_t nId)
{
	MPObjectWrapper<MPPeer*> m_Peer = NULL;
	
	m_oMutexPeers->lock();

	std::map<uint64_t, MPObjectWrapper<MPPeer*> >::iterator iter = m_Peers.find(nId);
	if(iter != m_Peers.end()){
		m_Peer = iter->second;
	}

	m_oMutexPeers->unlock();

	return m_Peer;
}

MPObjectWrapper<MPPeer*> MPEngine::getPeerBySessionId(uint64_t nId, bool bSessionLeft)
{
	m_oMutexPeers->lock();
	
	std::map<uint64_t, MPObjectWrapper<MPPeer*> >::iterator iter;
	MPObjectWrapper<MPPeer*> m_Peer = NULL;
	
	iter = bSessionLeft 
		? std::find_if(m_Peers.begin(), m_Peers.end(), std::bind2nd( _PeerBySessionIdLeft(), nId ))
		: std::find_if(m_Peers.begin(), m_Peers.end(), std::bind2nd( _PeerBySessionIdRight(), nId ));
	if(iter != m_Peers.end()){
		m_Peer = iter->second;
	}
	
	m_oMutexPeers->unlock();
	
	return m_Peer;
}

void MPEngine::insertPeer(MPObjectWrapper<MPPeer*> oPeer)
{
	if(oPeer){
		m_oMutexPeers->lock();
		m_Peers.insert( std::pair<uint64_t, MPObjectWrapper<MPPeer*> >(oPeer->getId(), oPeer) );
		m_oMutexPeers->unlock();
	}
}

void MPEngine::removePeer(uint64_t nId)
{
	m_oMutexPeers->lock();
	std::map<uint64_t, MPObjectWrapper<MPPeer*> >::iterator iter;
	if((iter = m_Peers.find(nId)) != m_Peers.end()){
		MPObjectWrapper<MPPeer*> oPeer = iter->second;
		m_Peers.erase(iter);
	}
	m_oMutexPeers->unlock();
}

MPObjectWrapper<MPEngine*> MPEngine::New()
{
	MPObjectWrapper<MPEngine*> oMPEngine = new MPEngine(MP_REALM, MP_IMPI, MP_IMPU);
	return oMPEngine;
}






}// namespace