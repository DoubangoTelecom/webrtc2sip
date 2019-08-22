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
#include "mp_engine.h"
#include "mp_proxyplugin_mgr.h"
#include "db/sqlite/mp_db_sqlite.h"

#include "SipStack.h"
#include "MediaSessionMgr.h"

#include <assert.h>
#include <functional>
#include <algorithm>

#if !defined(MP_USER_AGENT_STR)
#	define	MP_USER_AGENT_STR	"webrtc2sip Media Server " WEBRTC2SIP_VERSION_STRING
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
, m_pDtmfType(NULL)
, m_pC2CHttpDomain(NULL)
{
	if(!MPEngine::g_bInitialized){
		SipStack::initialize();
		MPProxyPluginMgr::initialize();
		MPEngine::g_bInitialized = true;
	}

	m_oMutex = MPMutex::New();
	m_oMutexPeers = MPMutex::New();

	m_SSL.pPrivateKey = m_SSL.pPublicKey = m_SSL.pCA = NULL;
	m_SSL.bVerify = false;

	if((m_oCallback = MPSipCallback::New(this)))
	{
		if((m_oSipStack = MPSipStack::New(m_oCallback, pRealmUri, pPrivateIdentity, pPublicIdentity)))
		{
			if(!const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setMode(tsip_stack_mode_webrtc2sip))
			{
				TSK_DEBUG_ERROR("SipStack::setModeServer failed");
				return;
			}
			const_cast<SipStack*>(m_oSipStack->getWrappedStack())->addHeader("User-Agent", MP_USER_AGENT_STR);
			m_bValid = true;
		}
	}

}

MPEngine::~MPEngine()
{
	m_oAccountSipCallers.clear();
	m_Peers.clear();
	m_C2CTransports.clear();

	TSK_FREE(m_SSL.pPrivateKey);
	TSK_FREE(m_SSL.pPublicKey);
	TSK_FREE(m_SSL.pCA);

	TSK_FREE(m_pDtmfType);

	TSK_FREE(m_pC2CHttpDomain);
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

bool MPEngine::addTransport(const char* pTransport, uint16_t nLocalPort, const char* pcLocalIP /*= tsk_null*/)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}

	if(tsk_striequals(pTransport, "c2c") || tsk_striequals(pTransport, "c2cs"))
	{
		bool isSecure = tsk_striequals(pTransport, "c2cs");
		MPObjectWrapper<MPC2CTransport*>oNetTransport = new MPC2CTransport(isSecure, pcLocalIP, nLocalPort);
		if(!oNetTransport){
			return false;
		}
		oNetTransport->setDb(m_oDb);
		m_C2CTransports.push_back(oNetTransport);
		return true;
	}
	else
	{
		bool bRet = const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setLocalIP(pcLocalIP, pTransport);
		bRet &= const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setLocalPort(nLocalPort, pTransport);
		return bRet;
	}
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

bool MPEngine::setRtpPortRange(uint16_t start, uint16_t stop)
{
	if(start < 1024 || stop < 1024 || start >= stop) {
        TSK_DEBUG_ERROR("Invalid parameter: (%u < 1024 || %u < 1024 || %u >= %u)", start, stop, start, stop);
        return false;
    }
	return  MediaSessionMgr::defaultsSetRtpPortRange(start, stop);
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
	return MediaSessionMgr::defaultsSetVideoJbEnabled(bEnabled);
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

bool MPEngine::setSSLCertificates(const char* pcPrivateKey, const char* pcPublicKey, const char* pcCA, bool bVerify /*= false*/)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}

	tsk_strupdate(&m_SSL.pPrivateKey, pcPrivateKey);
	tsk_strupdate(&m_SSL.pPublicKey, pcPublicKey);
	tsk_strupdate(&m_SSL.pCA, pcCA);
	m_SSL.bVerify = bVerify;

	return const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setSSLCertificates(
				pcPrivateKey,
				pcPublicKey,
				pcCA,
				bVerify
			);
}

bool MPEngine::setCodecs(const char* pcCodecs)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}

	int i;
	struct codec{ const char* name; tmedia_codec_id_t id; };
	static const codec aCodecNames[] = { 
		{"pcma", tmedia_codec_id_pcma}, 
		{"pcmu", tmedia_codec_id_pcmu},
		{"opus", tmedia_codec_id_opus},
		{"amr-nb-be", tmedia_codec_id_amr_nb_be},
		{"amr-nb-oa", tmedia_codec_id_amr_nb_oa},
		{"speex-nb", tmedia_codec_id_speex_nb}, 
		{"speex-wb", tmedia_codec_id_speex_wb}, 
		{"speex-uwb", tmedia_codec_id_speex_uwb}, 
		{"g729", tmedia_codec_id_g729ab}, 
		{"gsm", tmedia_codec_id_gsm}, 
		{"g722", tmedia_codec_id_g722}, 
		{"ilbc", tmedia_codec_id_ilbc},
		{"h264-bp", tmedia_codec_id_h264_bp}, 
		{"h264-mp", tmedia_codec_id_h264_mp}, 
		{"vp8", tmedia_codec_id_vp8}, 
		{"h263", tmedia_codec_id_h263}, 
		{"h263+", tmedia_codec_id_h263p}, 
		{"theora", tmedia_codec_id_theora}, 
		{"mp4v-es", tmedia_codec_id_mp4ves_es} 
	};
	static const int nCodecsCount = sizeof(aCodecNames) / sizeof(aCodecNames[0]);

	int64_t nCodecs = (int64_t)tmedia_codec_id_none;
	tsk_params_L_t* pParams;
	int nPriority = 0;

	if((pParams = tsk_params_fromstring(pcCodecs, ";", tsk_true)))
	{
		const tsk_list_item_t* item;
		tsk_list_foreach(item, pParams)
		{	
			const char* pcCodecName = ((const tsk_param_t*)item->data)->name;
			for(i = 0; i < nCodecsCount; ++i)
			{
				if(tsk_striequals(aCodecNames[i].name, pcCodecName))
				{
					nCodecs |= (int64_t)aCodecNames[i].id;
					if(!tdav_codec_is_supported((tdav_codec_id_t)aCodecNames[i].id)){
						TSK_DEBUG_INFO("'%s' codec enabled but not supported", aCodecNames[i].name);
					}
					else{
						tdav_codec_set_priority((tdav_codec_id_t)aCodecNames[i].id, nPriority++);
					}
					break;
				}
			}
		}
	}

	TSK_OBJECT_SAFE_FREE(pParams);

	const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setCodecs_2(nCodecs);
	return true;
}

bool MPEngine::setCodecOpusMaxRates(int32_t nPlaybackMaxRate, int32_t nCaptureMaxRate)
{
	return MediaSessionMgr::defaultsSetOpusMaxPlaybackRate(nPlaybackMaxRate) && MediaSessionMgr::defaultsSetOpusMaxCaptureRate(nCaptureMaxRate);
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

bool MPEngine::setDtmfType(const char* pcDtmfType)
{
	if(!pcDtmfType){
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}
	if(!tsk_striequals(pcDtmfType, MP_DTMF_TYPE_RFC2833) && !tsk_striequals(pcDtmfType, MP_DTMF_TYPE_RFC4733)){
		TSK_DEBUG_ERROR("%s not valid DTMF type", pcDtmfType);
		return false;
	}
	tsk_strupdate(&m_pDtmfType, pcDtmfType);
	return true;
}

bool MPEngine::setStunServer(const char* pcIP, unsigned short nPort, const char* pcUsrName, const char* pcUsrPwd)
{
	if(isValid())
	{
		const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setSTUNServer(pcIP, nPort);
		const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setSTUNCred(pcUsrName, pcUsrPwd);
	}
	
	return MediaSessionMgr::defaultsSetStunServer(pcIP, nPort) 
		&& 
		MediaSessionMgr::defaultsSetStunCred(pcUsrName, pcUsrPwd);
}

bool MPEngine::setIceStunEnabled(bool bEnabled)
{
	if(isValid())
	{
		const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setSTUNEnabledForICE(bEnabled);
	}
	return MediaSessionMgr::defaultsSetIceStunEnabled(bEnabled);
}

bool MPEngine::setMaxFds(int32_t nMaxFds)
{
	if (nMaxFds > 0)
	{
		bool ret;
		if ((ret = MediaSessionMgr::defaultsSetMaxFds(nMaxFds))) 
		{
			if(isValid())
			{
				ret = const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setMaxFDs(nMaxFds);
			}
		}
		return ret;
	}
	return true;
}

bool MPEngine::addDNSServer(const char* pcDNSServer)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}
	return const_cast<SipStack*>(m_oSipStack->getWrappedStack())->addDnsServer(pcDNSServer);
}

bool MPEngine::setDbInfo(const char* pcDbType, const char* pcDbConnectionInfo)
{
	if(m_oDb)
	{
		TSK_DEBUG_ERROR("Database information already defined");
		return false;
	}

	if(tsk_striequals(pcDbType, "sqlite"))
	{
		if(tsk_strnullORempty(pcDbConnectionInfo))
		{
			TSK_DEBUG_ERROR("Invalid parameter");
			return false;
		}
		m_oDb = new MPDbSQLite(pcDbConnectionInfo);		
		return true;
	}
	TSK_DEBUG_ERROR("%s not supported as valid database type");
	return false;
}

bool MPEngine::setHttpDomain(const char* pcC2CHttpDomain)
{
	tsk_strupdate(&m_pC2CHttpDomain, pcC2CHttpDomain);
	return true;
}

bool MPEngine::setRecaptchaInfo(const char* pcSiteVerifyUrl, const char* pcSecret)
{
	if(m_oRecaptchaTransport)
	{
		TSK_DEBUG_ERROR("Recaptcha info already defined");
		return false;
	}
	if(tsk_strnullORempty(pcSiteVerifyUrl) || tsk_strnullORempty(pcSecret))
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}
	m_oRecaptchaTransport = new MPRecaptchaTransport(pcSiteVerifyUrl, pcSecret);
	return !!m_oRecaptchaTransport;
}

bool MPEngine::setMailAccountInfo(const char* pcScheme, const char* pcLocalIP, unsigned short nLocalPort, const char* pcSmtpHost, unsigned short nSmtpPort, const char* pcEmail, const char* pcAuthName, const char* pcAuthPwd)
{
	bool bSecure = false;
	if(m_oMailTransport)
	{
		TSK_DEBUG_ERROR("Mail account info already defined");
		return false;
	}

	if(!pcSmtpHost || !nSmtpPort || !pcAuthName || !pcAuthPwd)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	if(!tsk_striequals(pcScheme, "smtp") && !(bSecure = tsk_striequals(pcScheme, "smtps")))
	{
		TSK_DEBUG_ERROR("%s not valid as SMTP scheme");
		return false;
	}

	m_oMailTransport = new MPMailTransport(bSecure, pcLocalIP, nLocalPort, pcSmtpHost, nSmtpPort, pcEmail, pcAuthName, pcAuthPwd);
	return !!m_oMailTransport;
}

bool MPEngine::addAccountSipCaller(const char* pcDisplayName, const char* pcImpu, const char* pcImpi, const char* pcRealm, const char* pcPassword)
{
	assert(pcImpu && pcImpi && pcRealm);
	// compute Ha1
	char *pStr = NULL;
	tsk_md5string_t ha1;
	tsk_sprintf(&pStr, "%s:%s:%s", pcImpi, pcRealm, pcPassword);
	tsk_md5compute(pStr, tsk_strlen(pStr), &ha1);

	MPObjectWrapper<MPDbAccountSipCaller*> oAccountSipCaller = new MPDbAccountSipCaller(
		MPDB_ACCOUNT_SIP_INVALID_ID,
		pcDisplayName,
		pcImpu,
		pcImpi,
		pcRealm,
		(const char*)ha1,
		MPDB_ACCOUNT_INVALID_ID);

	if(oAccountSipCaller)
	{
		m_oAccountSipCallers.push_back(oAccountSipCaller);
		return true;
	}
	return false;
}

MPObjectWrapper<MPDbAccountSipCaller*> MPEngine::findAccountSipCallerByRealm(const char* pcRealm)
{
	assert(pcRealm);

	std::list<MPObjectWrapper<MPDbAccountSipCaller*> >::iterator iter;
	iter = m_oAccountSipCallers.begin();
	for(; iter != m_oAccountSipCallers.end(); ++iter)
	{
		if(tsk_striequals((*iter)->getRealm(), pcRealm))
		{
			return (*iter);
		}
	}
	return NULL;
}

bool MPEngine::start()
{
	int ret = 0;
	std::list<MPObjectWrapper<MPC2CTransport*> >::iterator iter;

	m_oMutex->lock();

	if(isStarted())
	{
		goto bail;
	}

	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		ret = -1;
		goto bail;
	}

	// open Database
	if(m_oDb && !m_oDb->open())
	{
		return false;
	}

	// start all click2call transports
	iter = m_C2CTransports.begin();
	for(; iter != m_C2CTransports.end(); ++iter)
	{
		(*iter)->setDb(m_oDb);
		(*iter)->setMailTransport(m_oMailTransport);
		(*iter)->setRecaptchaTransport(m_oRecaptchaTransport);
		(*iter)->setSSLCertificates(m_SSL.pPrivateKey, m_SSL.pPublicKey, m_SSL.pCA, m_SSL.bVerify);
		(*iter)->setAllowedRemoteHost(m_pC2CHttpDomain);
		if((*iter)->start() != true)
		{
			ret = -3;
			goto bail;
		}
	}

	// start reCAPTCHA transport
	if (m_oRecaptchaTransport)
	{
		m_oRecaptchaTransport->setSSLCertificates(m_SSL.pPrivateKey, m_SSL.pPublicKey, m_SSL.pCA, m_SSL.bVerify);
		m_oRecaptchaTransport->setAllowedRemoteHost(m_pC2CHttpDomain);
		if(!m_oRecaptchaTransport->start())
		{
			ret = -4;
			goto bail;
		}
	}

	// start mail transport
	if(m_oMailTransport)
	{
		m_oMailTransport->setSSLCertificates(m_SSL.pPrivateKey, m_SSL.pPublicKey, m_SSL.pCA, m_SSL.bVerify);
		m_oMailTransport->setAllowedRemoteHost(m_pC2CHttpDomain);
		if(!m_oMailTransport->start())
		{
			ret = -5;
			goto bail;
		}
	}

	// start SIP stack
	if(const_cast<SipStack*>(m_oSipStack->getWrappedStack())->start())
	{
		//const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setRtpPortRange(this.rtp_port_start, this.rtp_port_stop);
		setStarted(true);
	}
	else
	{
		ret = -2;
		goto bail;
	}

bail:
	m_oMutex->unlock();

	if(ret != 0){
		TSK_DEBUG_ERROR("Failed to start SIP stack");
	}

	return (ret == 0);
}

bool MPEngine::stop()
{
	int ret = 0;
	std::list<MPObjectWrapper<MPC2CTransport*> >::iterator iter;

	m_oMutex->lock();

	if(!isStarted())
	{
		goto bail;
	}

	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		ret = -1;
		goto bail;
	}

	// close Database
	if(m_oDb)
	{
		m_oDb->close();
	}

	// stop all click2call transports
	iter = m_C2CTransports.begin();
	for(; iter != m_C2CTransports.end(); ++iter)
	{
		ret = (*iter)->stop();
	}

	// stop Mail transport
	if(m_oMailTransport)
	{
		ret = m_oMailTransport->stop();
	}

	// stop SIP stack
	if(const_cast<SipStack*>(m_oSipStack->getWrappedStack())->stop())
	{
		setStarted(false);
	}
	else
	{
		TSK_DEBUG_ERROR("Failed to stop SIP stack");
		ret = -2;
		goto bail;
	}

bail:
	m_oMutex->unlock();
	return (ret == 0);
}


bool MPEngine::setRtpPort(uint16_t start, uint16_t stop){
	return const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setRtpPortRange(start, stop);
}


uint16_t MPEngine::RtpPortStart(){
	return this->port_range_start;
}

uint16_t MPEngine::RtpPortStop(){
	return this->port_range_stop;
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