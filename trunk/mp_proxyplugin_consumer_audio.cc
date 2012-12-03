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
#include "mp_proxyplugin_consumer_audio.h"

#include "MediaSessionMgr.h"

namespace webrtc2sip {

//
//	MPProxyPluginConsumerAudio
//
MPProxyPluginConsumerAudio::MPProxyPluginConsumerAudio(uint64_t nId, const ProxyAudioConsumer* pcConsumer)
: MPProxyPlugin(MPMediaType_Audio, nId, dynamic_cast<const ProxyPlugin*>(pcConsumer)), m_pcWrappedConsumer(pcConsumer),
m_nPtime(MP_AUDIO_PTIME_DEFAULT),
m_nRate(MP_AUDIO_RATE_DEFAULT),
m_nChannels(MP_AUDIO_CHANNELS_DEFAULT)
{
	m_nVolumeComputeCount = 0;
	m_nVolume = 0.0;
	m_nLatency = 0;
	m_nLastPullTime = 0;
	m_pHeldBufferPtr = NULL;
	m_nHeldBufferSize = 0;
	m_nHeldBufferPos = 0;
	m_nLastRTPPacketTime = 0;
	m_bByPassDecoding = MediaSessionMgr::defaultsGetByPassDecoding();

	if((m_oCallback = new MPProxyPluginConsumerAudioCallback(this))){
		const_cast<ProxyAudioConsumer*>(m_pcWrappedConsumer)->setCallback(*m_oCallback);
	}
}

MPProxyPluginConsumerAudio::~MPProxyPluginConsumerAudio()
{
	m_pcWrappedConsumer = NULL;
	TSK_FREE(m_pHeldBufferPtr);

	TSK_DEBUG_INFO("MPProxyPluginConsumerAudio object destroyed");
}

int MPProxyPluginConsumerAudio::prepareCallback(int ptime, int rate, int channels)
{
	m_nPtime = ptime;
	m_nRate = rate;
	m_nChannels = channels;
	return MPProxyPlugin::prepare();
}

int MPProxyPluginConsumerAudio::startCallback()
{
	// call parent
	return MPProxyPlugin::start();
}

int MPProxyPluginConsumerAudio::pauseCallback()
{
	// call parent
	return MPProxyPlugin::pause();
}

int MPProxyPluginConsumerAudio::stopCallback()
{
	// call parent
	return MPProxyPlugin::stop();
}

int MPProxyPluginConsumerAudio::consumeCallback(const void* buffer_ptr, tsk_size_t buffer_size, const tsk_object_t* proto_hdr)
{
	if(m_oProducerOpposite && m_oProducerOpposite->isValid()){
		return m_oProducerOpposite->push(buffer_ptr, buffer_size);
	}
	return 0;
}

unsigned MPProxyPluginConsumerAudio::pullAndHold()
{
#if 0
	if(!isPrepared()){
		return 0;
	}

	if(!m_pHeldBufferPtr){
		m_nHeldBufferSize = (m_nRate * (MP_AUDIO_BITS_PER_SAMPLE_DEFAULT >> 3) * m_nChannels * m_nPtime)/1000;
		assert((m_pHeldBufferPtr = tsk_calloc(m_nHeldBufferSize, sizeof(uint8_t))));
	}
	
	bool bReset = false;
	if(m_nLastPullTime){
		m_nLatency += (MPTime::GetNow() - m_nLastPullTime) - m_nPtime;
		m_nLatency = TSK_CLAMP(0, m_nLatency, MP_AUDIO_MAX_LATENCY);
		
		// TSK_DEBUG_INFO("m_nLatency=%lld", m_nLatency);
		if(m_nLatency >= MP_AUDIO_MAX_LATENCY){
			m_nLatency = 0;
			bReset = true;
		}
	}

	m_nHeldBufferPos = Pull(m_pHeldBufferPtr, m_nHeldBufferSize);
	if(m_nHeldBufferPos){
		m_nVolumeComputeCount+=m_nPtime;
		if(m_nVolumeComputeCount >= 1000/* FIXME */){
			m_nVolumeComputeCount = 0;
			m_nVolume = 0;
		}
		int16_t* pHeldBufferPtr = (int16_t*)m_pHeldBufferPtr;
		double nValue = 0;
		for(uint32_t i = 0; i< m_nHeldBufferSize>>1; ++i){
			nValue += (pHeldBufferPtr[i] > 0) ? 20.0 * log10((double)TSK_ABS(pHeldBufferPtr[i]) / 32768.0) : 72.f;
		}
		nValue /= m_nHeldBufferSize>>1;
		m_nVolume = (m_nVolume + nValue)/(m_nVolumeComputeCount == 0 ? 1 : 2);
		//TSK_DEBUG_INFO("m_nVolume=%f", m_nVolume);
	}
	else{
		m_nVolume /= 2;
		//TSK_DEBUG_INFO("No Sound");
	}

	m_nLastPullTime = MPTime::GetNow();
	
	if(bReset){
		TSK_DEBUG_INFO("======BOUUUUH======");
	}

	if(m_nHeldBufferPos > 0){
		m_nLastRTPPacketTime = MPTime::GetNow();
	}

	return m_nHeldBufferPos;
#else
	return 0;
#endif
}

unsigned MPProxyPluginConsumerAudio::copyFromHeldBuffer(void* output, unsigned size)
{
	if(!output || !size){
		TSK_DEBUG_ERROR("Invalid parameter");
		return 0;
	}
	
	unsigned nRetSize = 0;

	if(m_pHeldBufferPtr){
		nRetSize = TSK_MIN(m_nHeldBufferPos, size);
		if(nRetSize){
			memcpy(output, m_pHeldBufferPtr, nRetSize);
		}
	}

	if(nRetSize < size){
		// complete with silence
		memset(((uint8_t*)output) + nRetSize, 0, (size - nRetSize));
	}

	return nRetSize;
}


} // namespace