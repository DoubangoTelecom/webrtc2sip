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
#if !defined(_MEDIAPROXY_PROXYPLUGIN_PRODUCER_VIDEO_H_)
#define _MEDIAPROXY_PROXYPLUGIN_PRODUCER_VIDEO_H_

#include "mp_config.h"
#include "mp_proxyplugin.h"

#include "ProxyProducer.h"

namespace webrtc2sip {


class MPProxyPluginProducerVideoCallback;

//
//	MPProxyPluginProducerVideo
//
class MPProxyPluginProducerVideo : public MPProxyPlugin
{
	friend class MPProxyPluginProducerVideoCallback;
public:
	MPProxyPluginProducerVideo(uint64_t nId, const ProxyVideoProducer* pcProducer);
	virtual ~MPProxyPluginProducerVideo();

	// Encode then send
	int push(const void* pcBuffer, unsigned nBufferSize, unsigned nFrameWidth, unsigned nFrameHeight);
	// Send "AS IS"
	int sendRaw(const void* pcBuffer, unsigned nSize, unsigned nDuration, bool bMarker){
		return m_pcWrappedProducer ? const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->sendRaw(pcBuffer, nSize, nDuration, bMarker) : 0;
	}
	int sendRaw(const void* pcBuffer, unsigned nSize, const void* pcProtoHdr){
		return m_pcWrappedProducer ? const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->sendRaw(pcBuffer, nSize, pcProtoHdr) : 0;
	}
	bool setActualCameraOutputSize(unsigned nWidth, unsigned nHeight){
		return m_pcWrappedProducer ? const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->setActualCameraOutputSize(nWidth, nHeight) : 0;
	}
	
	MP_INLINE int getFps() { return m_nFPS; }
	MP_INLINE int getHeight() { return m_nHeight; }
	MP_INLINE int getWidth() { return m_nWidth; }

	MP_INLINE void setByPassEncoding(bool bByPass){ m_bByPassEncoding = bByPass; }
	MP_INLINE bool isByPassEncoding(){ return m_bByPassEncoding; }

private:
	int prepareCallback(int nWidth, int nHeight, int nFps);
	int startCallback();
	int pauseCallback();
	int stopCallback();

private:
	MPObjectWrapper<MPProxyPluginProducerVideoCallback*> m_oCallback;
	const ProxyVideoProducer* m_pcWrappedProducer;
	int m_nWidth, m_nHeight, m_nFPS;
	bool m_bByPassEncoding;
};


//
//	MPProxyPluginProducerVideoCallback
//
class MPProxyPluginProducerVideoCallback : public ProxyVideoProducerCallback, public MPObject {
public:
	MPProxyPluginProducerVideoCallback(const MPProxyPluginProducerVideo* pcMPProducer)
	  : ProxyVideoProducerCallback(), m_pcMPProducer(pcMPProducer){
	}
	virtual ~MPProxyPluginProducerVideoCallback(){
	}
	MP_INLINE virtual const char* getObjectId() { return "MPProxyPluginProducerVideoCallback"; }

public: /* Overrides */
	virtual int prepare(int width, int height, int fps) { 
		return const_cast<MPProxyPluginProducerVideo*>(m_pcMPProducer)->prepareCallback(width, height, fps); 
	}
	virtual int start() { 
		return const_cast<MPProxyPluginProducerVideo*>(m_pcMPProducer)->startCallback(); 
	}
	virtual int pause() { 
		return const_cast<MPProxyPluginProducerVideo*>(m_pcMPProducer)->pauseCallback(); 
	}
	virtual int stop() { 
		return const_cast<MPProxyPluginProducerVideo*>(m_pcMPProducer)->stopCallback(); 
	}
private:
	const MPProxyPluginProducerVideo* m_pcMPProducer;
};


} // namespace
#endif /* _MEDIAPROXY_PROXYPLUGIN_PRODUCER_VIDEO_H_ */
