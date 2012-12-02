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
#if !defined(_MEDIAPROXY_PROXYPLUGIN_CONSUMER_VIDEO_H_)
#define _MEDIAPROXY_PROXYPLUGIN_CONSUMER_VIDEO_H_

#include "mp_config.h"
#include "mp_proxyplugin.h"
#include "mp_proxyplugin_producer_video.h"

#include "ProxyConsumer.h"

namespace webrtc2sip {

class MPProxyPluginConsumerVideoCallback;

class MPProxyPluginConsumerVideo : public MPProxyPlugin
{
	friend class MPProxyPluginConsumerVideoCallback;
public:
	MPProxyPluginConsumerVideo(uint64_t nId, const ProxyVideoConsumer* pcConsumer);
	virtual ~MPProxyPluginConsumerVideo();
	MP_INLINE virtual const char* getObjectId() { return "MPProxyPluginConsumerVideo"; }

public:
	MP_INLINE void setProducerOpposite(MPObjectWrapper<MPProxyPluginProducerVideo*> oProducer){
		m_oProducerOpposite = oProducer;
	}
	MP_INLINE bool setDisplaySize(int nWidth, int nHeight){ 
		return m_pcWrappedConsumer ? const_cast<ProxyVideoConsumer*>(m_pcWrappedConsumer)->setDisplaySize(nWidth, nHeight) : false;
	}
	MP_INLINE bool reset(){ 
		return m_pcWrappedConsumer ? const_cast<ProxyVideoConsumer*>(m_pcWrappedConsumer)->reset() : false;
	}
	MP_INLINE int getFps() { return m_nFPS; }
	MP_INLINE int getHeight() { return m_nHeight; }
	MP_INLINE int getWidth() { return m_nWidth; }
	MP_INLINE uint64_t getLastRTPPacketTime() { return m_nLastRTPPacketTime; }

	MP_INLINE void setByPassDecoding(bool bByPass){ m_bByPassDecoding = bByPass; }
	MP_INLINE bool isByPassDecoding(){ return m_bByPassDecoding; }

private:
	int prepareCallback(int nWidth, int nHeight, int nFps);
	int startCallback();
	int consumeCallback(const ProxyVideoFrame* frame);
	int pauseCallback();
	int stopCallback();

private:
	MPObjectWrapper<MPProxyPluginConsumerVideoCallback*> m_oCallback;
	// "Left" producer if consumer is on the "right" side and "Right" otherwise 
	MPObjectWrapper<MPProxyPluginProducerVideo*> m_oProducerOpposite;
	const ProxyVideoConsumer* m_pcWrappedConsumer;
	int m_nWidth, m_nHeight, m_nFPS;
	uint64_t m_nLastRTPPacketTime;
	bool m_bByPassDecoding;
};

//
//	MPProxyPluginConsumerVideoCallback
//
class MPProxyPluginConsumerVideoCallback : public ProxyVideoConsumerCallback, public MPObject {
public:
	MPProxyPluginConsumerVideoCallback(const MPProxyPluginConsumerVideo* pcConsumer)
	  : ProxyVideoConsumerCallback(), m_pcConsumer(pcConsumer){
	}
	virtual ~MPProxyPluginConsumerVideoCallback(){
	}
	MP_INLINE virtual const char* getObjectId() { return "MPProxyPluginConsumerVideoCallback"; }
public: /* Overrides */
	virtual int prepare(int width, int height, int fps) { 
		return const_cast<MPProxyPluginConsumerVideo*>(m_pcConsumer)->prepareCallback(width, height, fps); 
	}
	virtual int start() { 
		return const_cast<MPProxyPluginConsumerVideo*>(m_pcConsumer)->startCallback(); 
	}
	virtual int consume(const ProxyVideoFrame* frame) { 
		return const_cast<MPProxyPluginConsumerVideo*>(m_pcConsumer)->consumeCallback(frame); 
	}
	virtual int pause() { 
		return const_cast<MPProxyPluginConsumerVideo*>(m_pcConsumer)->pauseCallback(); 
	}
	virtual int stop() { 
		return const_cast<MPProxyPluginConsumerVideo*>(m_pcConsumer)->stopCallback(); 
	}
private:
	const MPProxyPluginConsumerVideo* m_pcConsumer;
};


} // namespace
#endif /* _MEDIAPROXY_PROXYPLUGIN_CONSUMER_VIDEO_H_ */
