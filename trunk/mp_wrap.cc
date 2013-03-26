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
#include "mp_wrap.h"
#include "mp_engine.h"
#include "mp_proxyplugin.h"
#include "mp_proxyplugin_mgr.h"
#include "mp_proxyplugin_consumer_audio.h"
#include "mp_proxyplugin_consumer_video.h"
#include "mp_proxyplugin_producer_audio.h"
#include "mp_proxyplugin_producer_video.h"
#include "db/mp_db_model.h"

#include "SipSession.h"
#include "SipEvent.h"
#include "ProxyPluginMgr.h"
#include "MediaSessionMgr.h"
#include "SipMessage.h"
#include "SipUri.h"

#include <assert.h>

#if !defined(MP_DTMF_CONTENT_TYPE)
#	define MP_DTMF_CONTENT_TYPE "application/dtmf-relay"
#endif /* MP_DTMF_CONTENT_TYPE */

namespace webrtc2sip {


//
//	MPSipCallback
//
MPSipCallback::MPSipCallback(MPObjectWrapper<MPEngine*> oEngine)
	:SipCallback() 
{
	m_oEngine = oEngine;
}

MPSipCallback::~MPSipCallback() 
{
	
}

void MPSipCallback::attachMediaProxyPlugins(MPObjectWrapper<MPPeer*> oPeer)
{
	const ProxyPlugin* pcProxyPlugin;
	MPObjectWrapper<MPProxyPlugin*> oMPProxyPlugin;
	MPObjectWrapper<MPProxyPluginProducerAudio*> oMPProxyPluginProducerAudioRight, oMPProxyPluginProducerAudioLeft;
	MPObjectWrapper<MPProxyPluginConsumerAudio*> oMPProxyPluginConsumerAudioRight, oMPProxyPluginConsumerAudioLeft;
	MPObjectWrapper<MPProxyPluginProducerVideo*> oMPProxyPluginProducerVideoRight, oMPProxyPluginProducerVideoLeft;
	MPObjectWrapper<MPProxyPluginConsumerVideo*> oMPProxyPluginConsumerVideoRight, oMPProxyPluginConsumerVideoLeft;
	const MediaSessionMgr *pcMediaSessionMgrRight, *pcMediaSessionMgrLeft;
	
	// make call to the right leg
	pcMediaSessionMgrRight = oPeer->getCallSessionRight() ? const_cast<CallSession*>(oPeer->getCallSessionRight()->getWrappedCallSession())->getMediaMgr() : NULL;
	pcMediaSessionMgrLeft = oPeer->getCallSessionLeft() ? const_cast<CallSession*>(oPeer->getCallSessionLeft()->getWrappedCallSession())->getMediaMgr() : NULL;
	
	// media session will be null if the invite is immediately rejected. do nothing as dialog terminated will be called
	if(pcMediaSessionMgrRight && pcMediaSessionMgrLeft){
		// find consumers and producers
		if((oPeer->getMediaType() & MPMediaType_Audio)){
			if((pcProxyPlugin = pcMediaSessionMgrLeft->findProxyPluginConsumer(twrap_media_audio)) && (oMPProxyPlugin = MPProxyPluginMgr::findPlugin(pcProxyPlugin->getId()))){
				oMPProxyPluginConsumerAudioLeft = *((MPObjectWrapper<MPProxyPluginConsumerAudio*>*)&oMPProxyPlugin);
			}
			else{
				TSK_DEBUG_ERROR("cannot find audio consumer for left leg");
			}
			if((pcProxyPlugin = pcMediaSessionMgrLeft->findProxyPluginProducer(twrap_media_audio)) && (oMPProxyPlugin = MPProxyPluginMgr::findPlugin(pcProxyPlugin->getId()))){
				oMPProxyPluginProducerAudioLeft = *((MPObjectWrapper<MPProxyPluginProducerAudio*>*)&oMPProxyPlugin);
			}
			else{
				TSK_DEBUG_ERROR("cannot find audio producer for left leg");
			}

			if((pcProxyPlugin = pcMediaSessionMgrRight->findProxyPluginConsumer(twrap_media_audio)) && (oMPProxyPlugin = MPProxyPluginMgr::findPlugin(pcProxyPlugin->getId()))){
				oMPProxyPluginConsumerAudioRight = *((MPObjectWrapper<MPProxyPluginConsumerAudio*>*)&oMPProxyPlugin);
			}
			else{
				TSK_DEBUG_ERROR("cannot find audio consumer for right leg");
			}
			if((pcProxyPlugin = pcMediaSessionMgrRight->findProxyPluginProducer(twrap_media_audio)) && (oMPProxyPlugin = MPProxyPluginMgr::findPlugin(pcProxyPlugin->getId()))){
				oMPProxyPluginProducerAudioRight = *((MPObjectWrapper<MPProxyPluginProducerAudio*>*)&oMPProxyPlugin);
			}
			else{
				TSK_DEBUG_ERROR("cannot find audio producer for right leg");
			}

			// attach consumers and producers
			if(oMPProxyPluginConsumerAudioLeft && oMPProxyPluginProducerAudioRight){ // Left -> Right
				oMPProxyPluginConsumerAudioLeft->setProducerOpposite(oMPProxyPluginProducerAudioRight);
			}
			if(oMPProxyPluginConsumerAudioRight && oMPProxyPluginProducerAudioLeft){ // Right -> Left
				oMPProxyPluginConsumerAudioRight->setProducerOpposite(oMPProxyPluginProducerAudioLeft);
			}
		}
		if((oPeer->getMediaType() & MPMediaType_Video)){
			if((pcProxyPlugin = pcMediaSessionMgrLeft->findProxyPluginConsumer(twrap_media_video)) && (oMPProxyPlugin = MPProxyPluginMgr::findPlugin(pcProxyPlugin->getId()))){
				oMPProxyPluginConsumerVideoLeft = *((MPObjectWrapper<MPProxyPluginConsumerVideo*>*)&oMPProxyPlugin);
			}
			else{
				TSK_DEBUG_ERROR("cannot find video consumer for left leg");
			}
			if((pcProxyPlugin = pcMediaSessionMgrLeft->findProxyPluginProducer(twrap_media_video)) && (oMPProxyPlugin = MPProxyPluginMgr::findPlugin(pcProxyPlugin->getId()))){
				oMPProxyPluginProducerVideoLeft = *((MPObjectWrapper<MPProxyPluginProducerVideo*>*)&oMPProxyPlugin);
			}
			else{
				TSK_DEBUG_ERROR("cannot find video producer for left leg");
			}

			if((pcProxyPlugin = pcMediaSessionMgrRight->findProxyPluginConsumer(twrap_media_video)) && (oMPProxyPlugin = MPProxyPluginMgr::findPlugin(pcProxyPlugin->getId()))){
				oMPProxyPluginConsumerVideoRight = *((MPObjectWrapper<MPProxyPluginConsumerVideo*>*)&oMPProxyPlugin);
			}
			else{
				TSK_DEBUG_ERROR("cannot find video consumer for right leg");
			}
			if((pcProxyPlugin = pcMediaSessionMgrRight->findProxyPluginProducer(twrap_media_video)) && (oMPProxyPlugin = MPProxyPluginMgr::findPlugin(pcProxyPlugin->getId()))){
				oMPProxyPluginProducerVideoRight = *((MPObjectWrapper<MPProxyPluginProducerVideo*>*)&oMPProxyPlugin);
			}
			else{
				TSK_DEBUG_ERROR("cannot find video producer for right leg");
			}

			// attach consumers and producers
			if(oMPProxyPluginConsumerVideoLeft && oMPProxyPluginProducerVideoRight){ // Left -> Right
				oMPProxyPluginConsumerVideoLeft->setProducerOpposite(oMPProxyPluginProducerVideoRight);
			}
			if(oMPProxyPluginConsumerVideoRight && oMPProxyPluginProducerVideoLeft){ // Right -> Left
				oMPProxyPluginConsumerVideoRight->setProducerOpposite(oMPProxyPluginProducerVideoLeft);
			}
		}
	}
	else{
		TSK_DEBUG_ERROR("Media sessions not ready yet");
	}
}

/* @override SipCallback::OnInviteEvent */
int MPSipCallback::OnInviteEvent(const InviteEvent* e)
{ 
	switch(e->getType())
	{
        default: break;

		/* INCOMING REQUEST */
		case tsip_i_request:
		{
			const SipMessage* pcSipMessage;
			const tsip_message_t* pcWrappedSipMessage;
			assert((pcSipMessage = e->getSipMessage()) && (pcWrappedSipMessage = pcSipMessage->getWrappedSipMessage()));			
			
			// Incoming INFO(dtmf-relay) ?
			if(TSIP_REQUEST_IS_INFO(pcWrappedSipMessage) && TSIP_MESSAGE_HAS_CONTENT(pcWrappedSipMessage) && tsk_striequals(TSIP_MESSAGE_CONTENT_TYPE(pcWrappedSipMessage), MP_DTMF_CONTENT_TYPE)){
				MPObjectWrapper<MPPeer*> oPeer;
				bool bFromLeftToRight = false;
				static const bool sIsLeftTrue = true;
				static const bool sIsLeftFalse = false;
				const InviteSession* pcInviteSession;
				
				// get the INVITE session associated to this event
				if(!(pcInviteSession = e->getSession()))
				{
					TSK_DEBUG_WARN("No INVITE session associated to this event");
					break;
				}
				uint64_t nSessionId = (uint64_t)pcInviteSession->getId();

				if((oPeer = m_oEngine->getPeerBySessionId(nSessionId, sIsLeftTrue)))
				{
					bFromLeftToRight = true;
				}
				else if(!(oPeer = m_oEngine->getPeerBySessionId(nSessionId, sIsLeftFalse)))
				{
					TSK_DEBUG_WARN("Failed to find peer hosting session with id = %llu", nSessionId);
					break;
				}
				
				const char* pcContent = (const char*)TSIP_MESSAGE_CONTENT_DATA(pcWrappedSipMessage);
				size_t nContentLength = (size_t)TSIP_MESSAGE_CONTENT_DATA_LENGTH(pcWrappedSipMessage);
				TSK_DEBUG_INFO("Processing INFO(dtmf-relay, %s->%s): %.*s", (bFromLeftToRight ? "Left" : "Right"), (bFromLeftToRight ? "Right" : "Left"), nContentLength, pcContent);
				if(pcContent && pcContent)
				{
					// find call session (receiver side)
					const CallSession* pcCallSession = bFromLeftToRight 
						? oPeer->getCallSessionRight()->getWrappedCallSession()
						: oPeer->getCallSessionLeft()->getWrappedCallSession();
					assert(pcCallSession);

					if(tsk_striequals(m_oEngine->m_pDtmfType, MP_DTMF_TYPE_RFC2833)) /* out-band (rfc2833) */
					{
						TSK_DEBUG_INFO("Gtw configured to relay DTMF using out-band(rfc2833) mode");
						if(!const_cast<CallSession*>(pcCallSession)->sendInfo(pcContent, nContentLength))
						{
							TSK_DEBUG_INFO("Failed to relay the DTMF digit - 'rfc2833'");
						}
					}
					else /* in-band (rfc4733) */
					{
						TSK_DEBUG_INFO("Gtw configured to relay DTMF using in-band(rfc4733) mode");

						int nSignalIndex = tsk_strindexOf(pcContent, nContentLength, "Signal=");
						if(nSignalIndex == -1)
						{
							TSK_DEBUG_ERROR("Failed to find 'Signal=' in the INFO content");
							break;
						}
						const char cSignal = pcContent[nSignalIndex + 7];

						// RFC 4733 code maping
						typedef struct dtmf_code_s { int code; char v; } dtmf_code_t;
						static const dtmf_code_t __dtmf_codes[16] = { {0, '0'}, {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'}, {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}, {9, '9'}, {10, '*'}, {11, '#'}, {12, 'A'}, {13, 'B'}, {14, 'C'}, {15, 'D'} };
						static const size_t __dtmf_codes_count = (sizeof(__dtmf_codes)/sizeof(__dtmf_codes[0]));
						// find the code
						size_t i;
						int dtmfCode = -1;
						for(i = 0; i<__dtmf_codes_count; ++i)
						{
							if(__dtmf_codes[i].v == cSignal)
							{
								dtmfCode = __dtmf_codes[i].code;
							}
						}
						if(dtmfCode == -1)
						{
							TSK_DEBUG_ERROR("Failed to map dtmf code with signal = '%c'", cSignal);
							break;
						}
						if(!const_cast<CallSession*>(pcCallSession)->sendDTMF(dtmfCode))
						{
							TSK_DEBUG_INFO("Failed to relay the DTMF digit - 'rfc4733'");
						}
					}
				}
			}

			break;
		}
            
		/* INCOMING NEW CALL (INVITE) */
		case tsip_i_newcall:
		{
			assert(e->getSession() == NULL);
			assert(e->getSipMessage() != NULL);

			#define MP_REJECT_CALL(pcSession, nCode, pcPhrase) \
			{ \
				ActionConfig* config = new ActionConfig(); \
				config->setResponseLine(nCode, pcPhrase); \
				const_cast<CallSession*>(pcSession)->reject(config); \
				if(config) delete config, config = tsk_null; \
			} \

			CallSession *pCallSessionLeft = NULL;
			if((pCallSessionLeft = e->takeCallSessionOwnership()))
			{
				// Before accepting the call "link it with the bridge"
				const MediaSessionMgr* pcMediaSessionMgrLeft = pCallSessionLeft->getMediaMgr();
				const SipMessage* pcMsgLeft = e->getSipMessage();
				assert(pcMsgLeft);

				const tsip_message_t* pcWrappedMsgLeft = const_cast<SipMessage*>(pcMsgLeft)->getWrappedSipMessage();
				static const bool g_bSessionLeft = true;
				bool bClick2Call = false;
				char *pDstUri = NULL, *pSrcUri = NULL, *pEmail = NULL, *pHa1 = NULL, *pImpi = NULL;
				MPObjectWrapper<MPSipSessionAV*> oCallSessionLeft;
				bool bHaveAudio = false, bHaveVideo = false;
				MPMediaType_t eMediaType = MPMediaType_None;


				// check whether it's a click2call request or not
				if((bClick2Call = tsk_striequals(tsk_params_get_param_value(pcWrappedMsgLeft->Contact->uri->params, "click2call"), "yes")))
				{
					const char* pcEmail64 = pcWrappedMsgLeft->From->uri->user_name; // base64 encoded email address
					const char* pcSipAddress64 = pcWrappedMsgLeft->To->uri->user_name; // base64 encoded SIP address
					TSK_DEBUG_INFO("click2call('%s' -> '%s')", pcEmail64, pcSipAddress64);
					// check database validity
					if(!m_oEngine->getDB())
					{
						TSK_DEBUG_ERROR("No database to look into for the click2call operation");
						MP_REJECT_CALL(pCallSessionLeft, 500, "No database to look into for the click2call operation");
						goto end_of_new_i_call;
					}
					// decode email
					tsk_base64_decode((const uint8_t*)pcEmail64, tsk_strlen(pcEmail64), &pEmail);
					// find user account (email is store without being encoded to speedup JSON processing)
					MPObjectWrapper<MPDbAccount*> oAccount= m_oEngine->getDB()->selectAccountByEmail(pEmail);
					if(!oAccount)
					{
						TSK_DEBUG_ERROR("Failed to find user account with eamil = %s", pEmail);
						MP_REJECT_CALL(pCallSessionLeft, 404, "Fail to find user account (click2call operation)");
						goto end_of_new_i_call;
					}
					// make sure the account is activated
					if(!oAccount->isActivated())
					{
						TSK_DEBUG_ERROR("Account with eamil = '%s' is not activated", pEmail);
						MP_REJECT_CALL(pCallSessionLeft, 403, "Account not activated (click2call operation)");
						goto end_of_new_i_call;
					}
					// find SIP account (SIP address is stored base64 encoded and is the email) - address could be the email (from==to)
					MPObjectWrapper<MPDbAccountSip*> oAccountSip = m_oEngine->getDB()->selectAccountSipByAccountIdAndAddress(oAccount->getId(), pcSipAddress64);
					if(!oAccountSip)
					{
						// probably (from==to) -> find first SIP address
						oAccountSip = m_oEngine->getDB()->selectAccountSipByAccountId(oAccount->getId());
						if(!oAccountSip)
						{
							TSK_DEBUG_ERROR("Failed to find SIP account with address = %s", pcSipAddress64);
							MP_REJECT_CALL(pCallSessionLeft, 404, "Fail to find SIP address (click2call operation)");
							goto end_of_new_i_call;
						}
					}
					// decode SIP address
					tsk_base64_decode((const uint8_t*)oAccountSip->getAddress64(), tsk_strlen(oAccountSip->getAddress64()), &pDstUri);
					TSK_DEBUG_INFO("click2call('%s' -> '%s')", pEmail, pDstUri);
					
					SipUri oDstUri(pDstUri);
					if(!oDstUri.isValid())
					{
						TSK_DEBUG_ERROR("Failed to parse SIP address = %s", pDstUri);
						MP_REJECT_CALL(pCallSessionLeft, 483, "Fail to parse SIP address (click2call operation)");
						goto end_of_new_i_call;
					}

					// find SIP caller from the database
					MPObjectWrapper<MPDbAccountSipCaller*> oAccountSipCaller = m_oEngine->getDB()->selectAccountSipCallerByAccountSipId(oAccountSip->getId());

					// find SIP caller from 'config.xml'
					if(!oAccountSipCaller)
					{
						oAccountSipCaller = m_oEngine->findAccountSipCallerByRealm(oDstUri.getHost());
						if(!oAccountSipCaller)
						{
							TSK_DEBUG_ERROR("Failed to find SIP caller for domain = %s", oDstUri.getHost());
							MP_REJECT_CALL(pCallSessionLeft, 404, "Fail to find SIP caller account (click2call operation)");
							goto end_of_new_i_call;
						}
					}
					pSrcUri = tsk_strdup(oAccountSipCaller->getImpu());
					pHa1 = tsk_strdup(oAccountSipCaller->getHa1());
					pImpi = tsk_strdup(oAccountSipCaller->getImpi());
				}
				else
				{
					pSrcUri = const_cast<SipMessage*>(pcMsgLeft)->getSipHeaderValue("f");
					pDstUri = const_cast<SipMessage*>(pcMsgLeft)->getSipHeaderValue("t");
					pHa1 = tsk_strdup(TSIP_HEADER_GET_PARAM_VALUE(pcWrappedMsgLeft->Contact, "ha1"));
					const char* pcImpi = TSIP_HEADER_GET_PARAM_VALUE(pcWrappedMsgLeft->Contact, "impi");
					if(pcImpi)
					{
						pImpi = tsk_url_decode(pcImpi);
					}
				}
				
				
				if(e->getMediaType() & twrap_media_audio){ eMediaType = (MPMediaType_t)(eMediaType | MPMediaType_Audio); bHaveAudio = true; }
				if(e->getMediaType() & twrap_media_video){ eMediaType = (MPMediaType_t)(eMediaType | MPMediaType_Video); bHaveVideo = true; }

				oCallSessionLeft = new MPSipSessionAV(&pCallSessionLeft, eMediaType);
				
				if(pDstUri && pcMediaSessionMgrLeft && (eMediaType != MPMediaType_None))
				{
					// find peer associated to this session (must be null)
					MPObjectWrapper<MPPeer*> oPeer = m_oEngine->getPeerBySessionId(oCallSessionLeft->getWrappedSession()->getId(), g_bSessionLeft);
					assert(!oPeer);
					
					oPeer = new MPPeer(oCallSessionLeft);
					oPeer->setSessionLeftState(MPPeerState_Connecting);
					oPeer->setMediaType(eMediaType);
					m_oEngine->insertPeer(oPeer);
					
					// make call to the right leg
					CallSession* pCallSessionRight = new CallSession(const_cast<SipStack*>(m_oEngine->m_oSipStack->getWrappedStack()));
					MPObjectWrapper<MPSipSessionAV*> oCallSessionRight = new MPSipSessionAV(&pCallSessionRight, eMediaType);
					oCallSessionRight->setSessionOpposite(oCallSessionLeft);
					oCallSessionLeft->setSessionOpposite(oCallSessionRight);
					const_cast<SipSession*>(oCallSessionRight->getWrappedSession())->setFromUri(pSrcUri);
					const_cast<SipSession*>(oCallSessionRight->getWrappedSession())->setAuth(pHa1, pImpi);
#if 0 // now using "ssrc.local" and "ssrc.remote"
					// use same SSRCs to make possible RTCP forwarding
					if(tsk_striequals("application/sdp", TSIP_MESSAGE_CONTENT_TYPE(pcWrappedMsgLeft))){
						tsdp_message_t* sdp_ro;
						if((sdp_ro = tsdp_message_parse(TSIP_MESSAGE_CONTENT_DATA(pcWrappedMsgLeft), TSIP_MESSAGE_CONTENT_DATA_LENGTH(pcWrappedMsgLeft)))){
							static const char* __ssrc_media_names[] = { "audio", "video" };
							static const twrap_media_type_t __ssrc_media_types[] = { twrap_media_audio, twrap_media_video };
							int ssrc_i;
							uint32_t ssrc;
							for(ssrc_i = 0; ssrc_i < sizeof(__ssrc_media_names)/sizeof(__ssrc_media_names[0]); ++ssrc_i){
								const tsdp_header_M_t* pcSdpM = tsdp_message_find_media(sdp_ro, __ssrc_media_names[ssrc_i]);
								if(pcSdpM){
									const tsdp_header_A_t* pcSdpAudioA = tsdp_header_M_findA(pcSdpM, "ssrc");
									if(pcSdpAudioA){
										if(sscanf(pcSdpAudioA->value, "%u %*s", &ssrc) != EOF){
											TSK_DEBUG_INFO("Using %s SSRC = %u", __ssrc_media_names[ssrc_i], ssrc);
											const_cast<CallSession*>(oCallSessionRight->getWrappedCallSession())->setMediaSSRC(__ssrc_media_types[ssrc_i], ssrc);
										}
									}
								}
							}
							TSK_OBJECT_SAFE_FREE(sdp_ro);
						}
					}
#endif
					
					// filter codecs if transcoding is disabled
					if(MediaSessionMgr::defaultsGetByPassEncoding()){
						int32_t nNegCodecs = const_cast<CallSession*>(oCallSessionLeft->getWrappedCallSession())->getNegotiatedCodecs();
						TSK_DEBUG_INFO("Negotiated codecs with the left leg = %d", nNegCodecs);
						const_cast<CallSession*>(oCallSessionRight->getWrappedCallSession())->setSupportedCodecs(nNegCodecs);
					}
					
					char* pRoute = tsk_null;

					// add destination based on the Request-Line
					const char* ws_src_ip = tsk_params_get_param_value (pcWrappedMsgLeft->line.request.uri->params, "ws-src-ip");
					const int ws_src_port = tsk_params_get_param_value_as_int(pcWrappedMsgLeft->line.request.uri->params, "ws-src-port");
					if(!tsk_strnullORempty(ws_src_ip) && ws_src_port > 0){
						const char* ws_src_proto = tsk_params_get_param_value(pcWrappedMsgLeft->line.request.uri->params, "ws-src-proto");
						tsk_sprintf(&pRoute, "<sip:%s:%d;transport=%s;lr>", ws_src_ip, ws_src_port, ws_src_proto ? ws_src_proto : "ws");
						const_cast<SipSession*>(oCallSessionRight->getWrappedSession())->addHeader("Route", pRoute);
						TSK_FREE(pRoute);
					}

					// add WebSocket src connection info in the AoR
					if(TNET_SOCKET_TYPE_IS_WS(pcWrappedMsgLeft->src_net_type) || TNET_SOCKET_TYPE_IS_WS(pcWrappedMsgLeft->src_net_type)){
						tnet_ip_t src_ip;
						tnet_port_t src_port;
						if(tnet_get_ip_n_port(pcWrappedMsgLeft->local_fd, tsk_false/*remote*/, &src_ip, &src_port) == 0){
							const char* src_proto = TNET_SOCKET_TYPE_IS_WS(pcWrappedMsgLeft->src_net_type) ? "ws" : "wss";
							const_cast<SipSession*>(oCallSessionRight->getWrappedSession())->setWebSocketSrc(src_ip, src_port, src_proto);
							tsk_sprintf(&pRoute, "<sip:%s:%d;transport=%s;lr>", src_ip, src_port, src_proto);
							const_cast<SipSession*>(oCallSessionLeft->getWrappedSession())->addHeader("Route", pRoute);
							TSK_FREE(pRoute);
						}
					}
					
					
					// add Routes
					int nRouteIndex = 0;
					while((pRoute = const_cast<SipMessage*>(pcMsgLeft)->getSipHeaderValue("route", nRouteIndex++))){
						const_cast<SipSession*>(oCallSessionRight->getWrappedSession())->addHeader("Route", pRoute);
						TSK_FREE(pRoute);
					}

					oPeer->setCallSessionRight(oCallSessionRight);
					oPeer->setSessionRightState(MPPeerState_Connecting);
					const_cast<CallSession*>(oPeer->getCallSessionRight()->getWrappedCallSession())->call(pDstUri, 
						(bHaveAudio && bHaveVideo) ? twrap_media_audio_video 
						: (bHaveAudio ? twrap_media_audio : twrap_media_video)
						);


					if(pCallSessionRight){
						delete pCallSessionRight, pCallSessionRight = NULL;
					}
				}
				else{
					MP_REJECT_CALL(pCallSessionLeft, 500, "Server error 501");
				}
end_of_new_i_call:
				if(pCallSessionLeft){
					delete pCallSessionLeft, pCallSessionLeft = NULL;
				}
				
				TSK_FREE(pDstUri);
				TSK_FREE(pSrcUri);
				TSK_FREE(pEmail);
				TSK_FREE(pHa1);
				TSK_FREE(pImpi);
			}
			break;
		}
	}
	return 0; 
}

/* @override SipCallback::OnDialogEvent */
int MPSipCallback::OnDialogEvent(const DialogEvent* e) 
{ 
	const SipSession *pcSession = e->getBaseSession();
	if(!pcSession){
		return 0;
	}
	const short code = e->getCode();
	const unsigned nId = pcSession->getId();

	switch(code)
	{
		case tsip_event_code_dialog_terminated:
		case tsip_event_code_dialog_terminating:
			{
				MPObjectWrapper<MPPeer*> oPeer = m_oEngine->getPeerBySessionId(nId, true);
				bool bIsleft = !!oPeer;
				
				if(!oPeer){
					oPeer = m_oEngine->getPeerBySessionId(nId, false);
				}

				if(oPeer){
					// Get SIP response code
					short nSipCode = 0;
					const char* pcPhrase = NULL;
					const SipMessage* pcMsg = e->getSipMessage();
					
					if(pcMsg && const_cast<SipMessage*>(pcMsg)->isResponse()){
						nSipCode = const_cast<SipMessage*>(pcMsg)->getResponseCode();
						pcPhrase = const_cast<SipMessage*>(pcMsg)->getResponsePhrase();
					}

					if(bIsleft){
						oPeer->setSessionLeftState(MPPeerState_Terminated);
						oPeer->setLastSipResponseLeft(nSipCode);
					}
					else{
						oPeer->setSessionRightState(MPPeerState_Terminated);
						oPeer->setLastSipResponseRight(nSipCode);
					}
					
					ActionConfig* config = new ActionConfig();
					if(config && nSipCode > 0){
						config->setResponseLine(nSipCode, pcPhrase);
					}
					
					if(oPeer->isSessionLeftActive() && oPeer->getCallSessionLeft()){
						const_cast<InviteSession*>(oPeer->getCallSessionLeft()->getWrappedInviteSession())->hangup(config);
					}
					if(oPeer->isSessionRightActive() && oPeer->getCallSessionRight()){
						const_cast<InviteSession*>(oPeer->getCallSessionRight()->getWrappedInviteSession())->hangup(config);
					}
					if(!oPeer->isSessionLeftActive() && !oPeer->isSessionRightActive()){
						m_oEngine->removePeer(oPeer->getId());
					}
					
					if(config){
						delete config, config = NULL;
					}
				}
				break;
			}
		case tsip_event_code_dialog_connected:
			{
				MPObjectWrapper<MPPeer*> oPeer = m_oEngine->getPeerBySessionId(nId, true);
				bool bIsleft = !!oPeer;

				if(!oPeer){
					oPeer = m_oEngine->getPeerBySessionId(nId, false);
				}

				if(oPeer){
					if(bIsleft){
						oPeer->setSessionLeftState(MPPeerState_Connected);
					}
					else{
						oPeer->setSessionRightState(MPPeerState_Connected);
						// right accepted the call -> accept left
						if(!oPeer->isSessionLeftConnected() && oPeer->getCallSessionLeft()){
							const_cast<InviteSession*>(oPeer->getCallSessionLeft()->getWrappedInviteSession())->accept();
						}
					}

					// attach media proxy plugins
					if(oPeer->isSessionLeftConnected() && oPeer->isSessionRightConnected()){
						attachMediaProxyPlugins(oPeer);
					}
				}
				break;
			}
	}

	

	return 0; 
}

/* @override SipCallback::OnRegistrationEvent */
int MPSipCallback::OnRegistrationEvent(const RegistrationEvent* e)
{ 
	switch(e->getType())
	{
        default: break;
            
		case tsip_i_newreg:
			{
				assert(e->getSession() == NULL);

				RegistrationSession *pRegSession;					
				if((pRegSession = e->takeSessionOwnership())){
					pRegSession->accept();
					delete pRegSession, pRegSession = NULL;
				}

				break;
			}
	}

	return 0; 
}

MPObjectWrapper<MPSipCallback*> MPSipCallback::New(MPObjectWrapper<MPEngine*> oEngine)
{
	MPObjectWrapper<MPSipCallback*> oCallback = new MPSipCallback(oEngine);
	return oCallback;
}



//
//	MPSipStack
//

MPSipStack::MPSipStack(MPObjectWrapper<MPSipCallback*> oCallback, const char* pRealmUri, const char* pPrivateId, const char* pPublicId)
{
	m_pStack = new SipStack(*oCallback, pRealmUri, pPrivateId, pPublicId);
}

MPSipStack::~MPSipStack()
{
	if(m_pStack){
		delete m_pStack, m_pStack = NULL;
	}
}

bool MPSipStack::isValid()
{
	return (m_pStack && m_pStack->isValid());
}

const SipStack* MPSipStack::getWrappedStack()
{
	return m_pStack;
}

MPObjectWrapper<MPSipStack*> MPSipStack::New(MPObjectWrapper<MPSipCallback*> oCallback, const char* pRealmUri, const char* pPrivateId, const char* pPublicId)
{
	MPObjectWrapper<MPSipStack*> oStack = new MPSipStack(oCallback, pRealmUri, pPrivateId, pPublicId);
	if(!oStack->isValid()){
		MPObjectSafeRelease(oStack);
	}
	return oStack;
}

}// namespace