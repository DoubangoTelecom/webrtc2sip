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
#include "mp_proxyplugin_producer_audio.h"

#include "MediaSessionMgr.h"

namespace webrtc2sip {

MPProxyPluginProducerAudio::MPProxyPluginProducerAudio(uint64_t nId, const ProxyAudioProducer* pcProducer)
: MPProxyPlugin(MPMediaType_Audio, nId, dynamic_cast<const ProxyPlugin*>(pcProducer)),
m_nPtime(MP_AUDIO_PTIME_DEFAULT),
m_nRate(MP_AUDIO_RATE_DEFAULT),
m_nChannels(MP_AUDIO_CHANNELS_DEFAULT)
{
	m_bByPassEncoding = MediaSessionMgr::defaultsGetByPassEncoding();
	m_pcWrappedProducer = pcProducer;
	m_oCallback = new MPProxyPluginProducerAudioCallback(this);
	const_cast<ProxyAudioProducer*>(m_pcWrappedProducer)->setCallback(*m_oCallback);
}

MPProxyPluginProducerAudio::~MPProxyPluginProducerAudio()
{
	m_pcWrappedProducer = NULL;

	TSK_DEBUG_INFO("MPProxyPluginProducerAudio object destroyed");
}

int MPProxyPluginProducerAudio::prepareCallback(int ptime, int rate, int channels)
{
	m_nPtime = ptime;
	m_nRate = rate;
	m_nChannels = channels;
	// call parent
	return MPProxyPlugin::prepare();
}

int MPProxyPluginProducerAudio::startCallback()
{
	
	// call parent
	return MPProxyPlugin::start();
}

int MPProxyPluginProducerAudio::pauseCallback()
{
	// call parent
	return MPProxyPlugin::pause();
}

int MPProxyPluginProducerAudio::stopCallback()
{
	int ret;

	// call parent
	if((ret = MPProxyPlugin::stop())){
		TSK_DEBUG_ERROR("Failed to stop producer");
		return ret;
	}

	return ret;
}

} // namespace