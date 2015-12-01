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
#include "mp_proxyplugin_consumer_video.h"

#include "MediaSessionMgr.h"

#include <assert.h>

namespace webrtc2sip {

MPProxyPluginConsumerVideo::MPProxyPluginConsumerVideo(uint64_t nId, const ProxyVideoConsumer* pcConsumer)
: MPProxyPlugin(MPMediaType_Video, nId, dynamic_cast<const ProxyPlugin*>(pcConsumer)), m_pcWrappedConsumer(pcConsumer),
m_nWidth(MP_VIDEO_WIDTH_DEFAULT), m_nHeight(MP_VIDEO_HEIGHT_DEFAULT), m_nFPS(MP_VIDEO_FPS_DEFAULT)
{
	m_nLastRTPPacketTime = 0;
	m_bByPassDecoding = MediaSessionMgr::defaultsGetByPassDecoding();

	m_oCallback = new MPProxyPluginConsumerVideoCallback(this);
	const_cast<ProxyVideoConsumer*>(m_pcWrappedConsumer)->setCallback(*m_oCallback);
	const_cast<ProxyVideoConsumer*>(m_pcWrappedConsumer)->setAutoResizeDisplay(true);
}

MPProxyPluginConsumerVideo::~MPProxyPluginConsumerVideo()
{
	m_pcWrappedConsumer = NULL;

	TSK_DEBUG_INFO("MPProxyPluginConsumerVideo object destroyed");
}

int MPProxyPluginConsumerVideo::prepareCallback(int nWidth, int nHeight, int nFps)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nFPS = nFps;

	// call parent
	return MPProxyPlugin::prepare();
}

int MPProxyPluginConsumerVideo::startCallback()
{
	// call parent
	return MPProxyPlugin::start();
}

int MPProxyPluginConsumerVideo::consumeCallback(const ProxyVideoFrame* frame)
{
	if(frame && m_oProducerOpposite && m_oProducerOpposite->isValid()){
		if(m_oProducerOpposite->isByPassEncoding()){
			return m_oProducerOpposite->sendRaw(frame->getBufferPtr(),
				frame->getBufferSize(),
				frame->getProtoHdr());
		}
		else{
			return m_oProducerOpposite->push(
				frame->getBufferPtr(),
				frame->getBufferSize(),
				frame->getFrameWidth(),
				frame->getFrameHeight());
		}
	}
	return 0;
}

int MPProxyPluginConsumerVideo::pauseCallback()
{
	// call parent
	return MPProxyPlugin::pause();
}

int MPProxyPluginConsumerVideo::stopCallback()
{
	// call parent
	return MPProxyPlugin::stop();
}


} // namespace
