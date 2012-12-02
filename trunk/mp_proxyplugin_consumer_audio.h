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
#if !defined(_MEDIAPROXY_PROXYPLUGIN_CONSUMER_AUDIO_H_)
#define _MEDIAPROXY_PROXYPLUGIN_CONSUMER_AUDIO_H_

#include "mp_config.h"
#include "mp_proxyplugin.h"
#include "mp_proxyplugin_producer_audio.h"

#include "ProxyConsumer.h"

namespace webrtc2sip {

class MPProxyPluginConsumerAudioCallback;

//
//	MPProxyPluginConsumerAudio
//
class MPProxyPluginConsumerAudio : public MPProxyPlugin
{
	friend class MPProxyPluginConsumerAudioCallback;
public:
	MPProxyPluginConsumerAudio(uint64_t nId, const ProxyAudioConsumer* pcConsumer);
	virtual ~MPProxyPluginConsumerAudio();

	MP_INLINE void setProducerOpposite(MPObjectWrapper<MPProxyPluginProducerAudio*> oProducer){
		m_oProducerOpposite = oProducer;
	}
	MP_INLINE unsigned pull(void* output, unsigned size){
		return m_pcWrappedConsumer ? const_cast<ProxyAudioConsumer*>(m_pcWrappedConsumer)->pull(output, size) : 0;
	}
	unsigned pullAndHold();
	unsigned copyFromHeldBuffer(void* output, unsigned size);
	MP_INLINE bool reset(){
		return m_pcWrappedConsumer ? const_cast<ProxyAudioConsumer*>(m_pcWrappedConsumer)->reset() : false;
	}
	MP_INLINE int getPtime() { return m_nPtime; }
	MP_INLINE int getRate() { return m_nRate; }
	MP_INLINE int getChannels() { return m_nChannels; }
	MP_INLINE double getVolume() { return m_nVolume; }
	MP_INLINE uint64_t getLastRTPPacketTime() { return m_nLastRTPPacketTime; }

	MP_INLINE void setByPassDecoding(bool bByPass){ m_bByPassDecoding = bByPass; }
	MP_INLINE bool isByPassDecoding(){ return m_bByPassDecoding; }

private:
	int prepareCallback(int ptime, int rate, int channels);
	int startCallback();
	int pauseCallback();
	int stopCallback();
	int consumeCallback(const void* buffer_ptr, tsk_size_t buffer_size, const tsk_object_t* proto_hdr);

private:
	MPObjectWrapper<MPProxyPluginConsumerAudioCallback*> m_oCallback;
	// "Left" producer if consumer is on the "right" side and "Right" otherwise 
	MPObjectWrapper<MPProxyPluginProducerAudio*> m_oProducerOpposite;
	const ProxyAudioConsumer* m_pcWrappedConsumer;
	int m_nPtime;
	int m_nRate;
	int m_nChannels;
	void* m_pHeldBufferPtr;
	double m_nVolume;
	uint32_t m_nVolumeComputeCount;
	uint32_t m_nHeldBufferSize;
	uint32_t m_nHeldBufferPos;
	int64_t m_nLatency;
	uint64_t m_nLastPullTime;
	uint64_t m_nLastRTPPacketTime;
	bool m_bByPassDecoding;
};

//
//	MPProxyPluginConsumerAudioCallback
//
class MPProxyPluginConsumerAudioCallback : public ProxyAudioConsumerCallback, public MPObject {
public:
	MPProxyPluginConsumerAudioCallback(const MPProxyPluginConsumerAudio* pcConsumer)
	  : ProxyAudioConsumerCallback(), m_pcConsumer(pcConsumer){
	}
	virtual ~MPProxyPluginConsumerAudioCallback(){
	}
	MP_INLINE virtual const char* getObjectId() { return "MPProxyPluginConsumerAudioCallback"; }
public: /* Overrides */
	virtual int prepare(int ptime, int rate, int channels) { 
		return const_cast<MPProxyPluginConsumerAudio*>(m_pcConsumer)->prepareCallback(ptime, rate, channels); 
	}
	virtual int start() { 
		return const_cast<MPProxyPluginConsumerAudio*>(m_pcConsumer)->startCallback();
	}
	virtual int pause() { 
		return const_cast<MPProxyPluginConsumerAudio*>(m_pcConsumer)->pauseCallback();
	}
	virtual int stop() { 
		return const_cast<MPProxyPluginConsumerAudio*>(m_pcConsumer)->stopCallback();
	}
	virtual bool putInJitterBuffer(){ 
		return false; 
	}
	virtual int consume(const void* buffer_ptr, tsk_size_t buffer_size, const tsk_object_t* proto_hdr) { 
		return const_cast<MPProxyPluginConsumerAudio*>(m_pcConsumer)->consumeCallback(buffer_ptr, buffer_size, proto_hdr);
	}
private:
	const MPProxyPluginConsumerAudio* m_pcConsumer;
};

} // namespace
#endif /* _MEDIAPROXY_PROXYPLUGIN_CONSUMER_AUDIO_H_ */
