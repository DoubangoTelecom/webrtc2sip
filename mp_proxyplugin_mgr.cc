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
#include "mp_proxyplugin_mgr.h"
#include "mp_proxyplugin_consumer_audio.h"
#include "mp_proxyplugin_consumer_video.h"
#include "mp_proxyplugin_producer_audio.h"
#include "mp_proxyplugin_producer_video.h"

#include "SipStack.h"
#include "MediaSessionMgr.h"

#include <assert.h>

using namespace std;

namespace webrtc2sip {

ProxyPluginMgr* MPProxyPluginMgr::g_pPluginMgr = NULL;
MPProxyPluginMgrCallback* MPProxyPluginMgr::g_pPluginMgrCallback = NULL;
MPMapOfPlugins* MPProxyPluginMgr::g_pPlugins = NULL;
tsk_mutex_handle_t* MPProxyPluginMgr::g_phMutex = NULL;
bool MPProxyPluginMgr::g_bInitialized = false;

//
//	MPProxyPluginMgr
//
void MPProxyPluginMgr::initialize()
{
	if(!MPProxyPluginMgr::g_bInitialized)
	{
		ProxyAudioConsumer::registerPlugin();
		ProxyAudioProducer::registerPlugin();
		ProxyVideoProducer::registerPlugin();
		ProxyVideoConsumer::registerPlugin();

		ProxyVideoConsumer::setDefaultChroma(tmedia_chroma_yuv420p);
		ProxyVideoProducer::setDefaultChroma(tmedia_chroma_yuv420p);
		// do not try to resize incoming video packets to match negotiated size
		ProxyVideoConsumer::setDefaultAutoResizeDisplay(true);

		// up to the remote peers
		MediaSessionMgr::defaultsSetEchoSuppEnabled(false);
		MediaSessionMgr::defaultsSetAgcEnabled(false);
		MediaSessionMgr::defaultsSetNoiseSuppEnabled(false);

		MediaSessionMgr::defaultsSet100relEnabled(false);

		// SRTP options
		MediaSessionMgr::defaultsSetSRtpMode(tmedia_srtp_mode_optional);
		MediaSessionMgr::defaultsSetSRtpType(tmedia_srtp_type_sdes_dtls);

		// set preferred video size
		MediaSessionMgr::defaultsSetPrefVideoSize(tmedia_pref_video_size_vga);

		// do not transcode
		MediaSessionMgr::defaultsSetByPassEncoding(true);
		MediaSessionMgr::defaultsSetByPassDecoding(true);

		// disable video jitter buffer
		MediaSessionMgr::defaultsSetVideoJbEnabled(false);

		// enlarge RTP network buffer size
		MediaSessionMgr::defaultsSetRtpBuffSize(0xffff);

		// NATT traversal options
		MediaSessionMgr::defaultsSetRtpSymetricEnabled(true);
		MediaSessionMgr::defaultsSetIceEnabled(true);

		// enlarge AVPF tail to honor more RTCP-NACK requests
		MediaSessionMgr::defaultsSetAvpfTail(100, 400);

		// default codecs (same as what said in config.xml)
		SipStack::setCodecs_2(
				(tmedia_codec_id_pcma |
				tmedia_codec_id_pcmu |
				tmedia_codec_id_gsm |
				tmedia_codec_id_vp8 |
				tmedia_codec_id_h264_bp |
				tmedia_codec_id_h264_mp |
				tmedia_codec_id_h263 |
				tmedia_codec_id_h263p)
		);

		MPProxyPluginMgr::g_pPluginMgrCallback = new MPProxyPluginMgrCallback();
		MPProxyPluginMgr::g_pPluginMgr = ProxyPluginMgr::createInstance(MPProxyPluginMgr::g_pPluginMgrCallback);
		assert(MPProxyPluginMgr::g_pPluginMgr);
		MPProxyPluginMgr::g_pPlugins = new MPMapOfPlugins();
		MPProxyPluginMgr::g_phMutex = tsk_mutex_create_2(tsk_false);
		MPProxyPluginMgr::g_bInitialized = true;
	}
}

void MPProxyPluginMgr::deInitialize()
{
	if(MPProxyPluginMgr::g_bInitialized)
	{
		if(MPProxyPluginMgr::g_phMutex){
			tsk_mutex_destroy(&MPProxyPluginMgr::g_phMutex);
		}
		if(MPProxyPluginMgr::g_pPlugins){
			delete MPProxyPluginMgr::g_pPlugins, MPProxyPluginMgr::g_pPlugins = NULL;
		}
		if(MPProxyPluginMgr::g_pPluginMgrCallback){
			delete MPProxyPluginMgr::g_pPluginMgrCallback, MPProxyPluginMgr::g_pPluginMgrCallback = NULL;
		}
		ProxyPluginMgr::destroyInstance(&MPProxyPluginMgr::g_pPluginMgr);

		MPProxyPluginMgr::g_bInitialized = false;
	}
}

//
//	MPProxyPluginMgrCallback
//
MPProxyPluginMgrCallback::MPProxyPluginMgrCallback()
:ProxyPluginMgrCallback()
{
}

MPProxyPluginMgrCallback::~MPProxyPluginMgrCallback()
{
}

// @Override
int MPProxyPluginMgrCallback::OnPluginCreated(uint64_t id, enum twrap_proxy_plugin_type_e type)
{
	tsk_mutex_lock(MPProxyPluginMgr::getMutex());

	switch(type){
		case twrap_proxy_plugin_audio_producer:
			{
				const ProxyAudioProducer* pcProducer = MPProxyPluginMgr::getPluginMgr()->findAudioProducer(id);
				if(pcProducer){
					MPObjectWrapper<MPProxyPlugin*> pMPProducer = new MPProxyPluginProducerAudio(id, pcProducer);
					MPProxyPluginMgr::getPlugins()->insert( pair<uint64_t, MPObjectWrapper<MPProxyPlugin*> >(id, pMPProducer) );
				}
				break;
			}
		case twrap_proxy_plugin_video_producer:
			{
				const ProxyVideoProducer* pcProducer = MPProxyPluginMgr::getPluginMgr()->findVideoProducer(id);
				if(pcProducer){
					MPObjectWrapper<MPProxyPlugin*> pMPProducer = new MPProxyPluginProducerVideo(id, pcProducer);
					MPProxyPluginMgr::getPlugins()->insert( pair<uint64_t, MPObjectWrapper<MPProxyPlugin*> >(id, pMPProducer) );
				}
				break;
			}
		case twrap_proxy_plugin_audio_consumer:
			{
				const ProxyAudioConsumer* pcConsumer = MPProxyPluginMgr::getPluginMgr()->findAudioConsumer(id);
				if(pcConsumer){
					MPObjectWrapper<MPProxyPlugin*> oMPConsumer = new MPProxyPluginConsumerAudio(id, pcConsumer);
					MPProxyPluginMgr::getPlugins()->insert( pair<uint64_t, MPObjectWrapper<MPProxyPlugin*> >(id, oMPConsumer) );
				}
				break;
			}
		case twrap_proxy_plugin_video_consumer:
			{
				const ProxyVideoConsumer* pcConsumer = MPProxyPluginMgr::getPluginMgr()->findVideoConsumer(id);
				if(pcConsumer){
					MPObjectWrapper<MPProxyPlugin*> pMPConsumer = new MPProxyPluginConsumerVideo(id, pcConsumer);
					MPProxyPluginMgr::getPlugins()->insert( pair<uint64_t, MPObjectWrapper<MPProxyPlugin*> >(id, pMPConsumer) );
				}
				break;
			}
		default:
			{
				assert(0);
				break;
			}
	}

	tsk_mutex_unlock(MPProxyPluginMgr::getMutex());

	return 0;
}

// @Override
int MPProxyPluginMgrCallback::OnPluginDestroyed(uint64_t id, enum twrap_proxy_plugin_type_e type)
{
	tsk_mutex_lock(MPProxyPluginMgr::getMutex());

	switch(type){
		case twrap_proxy_plugin_audio_producer:
		case twrap_proxy_plugin_video_producer:
		case twrap_proxy_plugin_audio_consumer:
		case twrap_proxy_plugin_video_consumer:
		{
			MPProxyPluginMgr::erasePlugin(id);
			break;
		}
		default:
		{
			assert(0);
			break;
		}
	}

	tsk_mutex_unlock(MPProxyPluginMgr::getMutex());

	return 0;
}



} // namespace