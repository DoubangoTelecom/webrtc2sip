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
#if !defined(_MEDIAPROXY_PROXYPLUGIN_PRODUCER_AUDIO_H_)
#define _MEDIAPROXY_PROXYPLUGIN_PRODUCER_AUDIO_H_

#include "mp_config.h"
#include "mp_proxyplugin.h"

#include "ProxyProducer.h"

namespace webrtc2sip {

class MPProxyPluginProducerAudioCallback;

//
//	MPProxyPluginProducerAudio
//
class MPProxyPluginProducerAudio : public MPProxyPlugin
{
	friend class MPProxyPluginProducerAudioCallback;
public:
	MPProxyPluginProducerAudio(uint64_t nId, const ProxyAudioProducer* pcProducer);
	virtual ~MPProxyPluginProducerAudio();
	int push(const void* buffer, unsigned size, int32_t nPtime, int32_t nRate, int32_t nChannels){ 
		if(m_bStarted){
			if(m_nPtime != nPtime || m_nRate != nRate || m_nChannels != nChannels){
				TSK_DEBUG_INFO("Consumer audio params different than producer expected input...request resampler");
				m_nPtime = nPtime;
				m_nRate = nRate;
				m_nChannels = nChannels;
				if(!const_cast<ProxyAudioProducer*>(m_pcWrappedProducer)->setActualSndCardRecordParams(m_nPtime, m_nRate, m_nChannels)){
					TSK_DEBUG_ERROR("setActualSndCardRecordParams(%d, %d, %d) failed", nPtime, nRate, nChannels);
					return -1;
				}
			}
			return (m_pcWrappedProducer ? const_cast<ProxyAudioProducer*>(m_pcWrappedProducer)->push(buffer, size) : -1);
		}
		TSK_DEBUG_INFO("Audio producer not started yet");
		return 0;
	}
	MP_INLINE int getPtime() { return m_nPtime; }
	MP_INLINE int getRate() { return m_nRate; }
	MP_INLINE int getChannels() { return m_nChannels; }

	MP_INLINE void setByPassEncoding(bool bByPass){ m_bByPassEncoding = bByPass; }
	MP_INLINE bool isByPassEncoding(){ return m_bByPassEncoding; }

private:
	int prepareCallback(int ptime, int rate, int channels);
	int startCallback();
	int pauseCallback();
	int stopCallback();

private:
	MPObjectWrapper<MPProxyPluginProducerAudioCallback*> m_oCallback;
	const ProxyAudioProducer* m_pcWrappedProducer;
	int m_nPtime;
	int m_nRate;
	int m_nChannels;
	bool m_bByPassEncoding;
};


//
//	MPProxyPluginProducerAudioCallback
//
class MPProxyPluginProducerAudioCallback : public ProxyAudioProducerCallback, public MPObject {
public:
	MPProxyPluginProducerAudioCallback(const MPProxyPluginProducerAudio* pcMPProducer)
	  : ProxyAudioProducerCallback(), m_pcMPProducer(pcMPProducer){
	}
	virtual ~MPProxyPluginProducerAudioCallback(){
	}
	MP_INLINE virtual const char* getObjectId() { return "MPProxyPluginProducerAudioCallback"; }

public: /* Overrides */
	virtual int prepare(int ptime, int rate, int channels) { 
		return const_cast<MPProxyPluginProducerAudio*>(m_pcMPProducer)->prepareCallback(ptime, rate, channels);
	}
	virtual int start() { 
		return const_cast<MPProxyPluginProducerAudio*>(m_pcMPProducer)->startCallback(); 
	}
	virtual int pause() { 
		return const_cast<MPProxyPluginProducerAudio*>(m_pcMPProducer)->pauseCallback(); 
	}
	virtual int stop() { 
		return const_cast<MPProxyPluginProducerAudio*>(m_pcMPProducer)->stopCallback(); 
	}
private:
	const MPProxyPluginProducerAudio* m_pcMPProducer;
};


} // namespace
#endif /* _MEDIAPROXY_PROXYPLUGIN_PRODUCER_AUDIO_H_ */
