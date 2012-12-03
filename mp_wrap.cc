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

#include "SipSession.h"
#include "SipEvent.h"
#include "ProxyPluginMgr.h"
#include "MediaSessionMgr.h"
#include "SipMessage.h"

#include <assert.h>

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
		case tsip_i_newcall:
		{
			assert(e->getSession() == NULL);
			assert(e->getSipMessage() != NULL);

			CallSession *pCallSessionLeft = NULL;
			if((pCallSessionLeft = e->takeCallSessionOwnership())){
				// Before accepting the call "link it with the bridge"
				const MediaSessionMgr* pcMediaSessionMgrLeft = pCallSessionLeft->getMediaMgr();
				const SipMessage* pcMsgLeft = e->getSipMessage();
				assert(pcMsgLeft);

				const tsip_message_t* pcWrappedMsgLeft = const_cast<SipMessage*>(pcMsgLeft)->getWrappedSipMessage();
				static const bool g_bSessionLeft = true;
				char* pDstUri = const_cast<SipMessage*>(pcMsgLeft)->getSipHeaderValue("t");
				char* pSrcUri = const_cast<SipMessage*>(pcMsgLeft)->getSipHeaderValue("f");
				bool bHaveAudio = false, bHaveVideo = false;

				MPMediaType_t eMediaType = MPMediaType_None;
				if(e->getMediaType() & twrap_media_audio){ eMediaType = (MPMediaType_t)(eMediaType | MPMediaType_Audio); bHaveAudio = true; }
				if(e->getMediaType() & twrap_media_video){ eMediaType = (MPMediaType_t)(eMediaType | MPMediaType_Video); bHaveVideo = true; }

				MPObjectWrapper<MPSipSessionAV*> oCallSessionLeft = new MPSipSessionAV(&pCallSessionLeft, eMediaType);
				
				if(pDstUri && pcMediaSessionMgrLeft && (eMediaType != MPMediaType_None)){
					// find peer associated to this session (must be null)
					MPObjectWrapper<MPPeer*> oPeer = m_oEngine->getPeerBySessionId(oCallSessionLeft->getWrappedSession()->getId(), g_bSessionLeft);
					assert(!oPeer);
					
					oPeer = new MPPeer(oCallSessionLeft);
					oPeer->setSessionLeftState(MPPeerState_Connecting);
					oPeer->setMediaType(eMediaType);
					m_oEngine->insertPeer(oPeer);
					
					// make call to the right leg
					CallSession* pCallSessionRight = new CallSession(const_cast<SipStack*>(m_oEngine->m_oStack->getWrappedStack()));
					MPObjectWrapper<MPSipSessionAV*> oCallSessionRight = new MPSipSessionAV(&pCallSessionRight, eMediaType);
					oCallSessionRight->setSessionOpposite(oCallSessionLeft);
					oCallSessionLeft->setSessionOpposite(oCallSessionRight);
					const_cast<SipSession*>(oCallSessionRight->getWrappedSession())->setFromUri(pSrcUri);

					// authentication token
					const char *pcHa1, *pcIMPI;
					if((pcHa1 = TSIP_HEADER_GET_PARAM_VALUE(pcWrappedMsgLeft->Contact, "ha1")) && (pcIMPI = TSIP_HEADER_GET_PARAM_VALUE(pcWrappedMsgLeft->Contact, "impi"))){
						const_cast<SipSession*>(oCallSessionRight->getWrappedSession())->setAuth(pcHa1, pcIMPI);
					}
					
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
											TSK_DEBUG_INFO("Using audio SSRC = %u", ssrc);
											const_cast<CallSession*>(oCallSessionRight->getWrappedCallSession())->setMediaSSRC(__ssrc_media_types[ssrc_i], ssrc);
										}
									}
								}
							}
							TSK_OBJECT_SAFE_FREE(sdp_ro);
						}
					}
					
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
					ActionConfig* config = new ActionConfig();
					config->setResponseLine(500, "Server error 501");
					const_cast<InviteSession*>(oCallSessionLeft->getWrappedInviteSession())->reject(config);
					if(config) delete config, config = tsk_null;
				}

				if(pCallSessionLeft){
					delete pCallSessionLeft, pCallSessionLeft = NULL;
				}

				TSK_FREE(pDstUri);
				TSK_FREE(pSrcUri);
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