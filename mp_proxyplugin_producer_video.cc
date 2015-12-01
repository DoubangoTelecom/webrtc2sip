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
#include "mp_proxyplugin_producer_video.h"

#include "MediaSessionMgr.h"

namespace webrtc2sip {


MPProxyPluginProducerVideo::MPProxyPluginProducerVideo(uint64_t nId, const ProxyVideoProducer* pcProducer)
: MPProxyPlugin(MPMediaType_Video, nId, dynamic_cast<const ProxyPlugin*>(pcProducer)),
m_nWidth(MP_VIDEO_WIDTH_DEFAULT), m_nHeight(MP_VIDEO_HEIGHT_DEFAULT), m_nFPS(MP_VIDEO_FPS_DEFAULT)
{
	m_bByPassEncoding = MediaSessionMgr::defaultsGetByPassEncoding();
	m_pcWrappedProducer = pcProducer;
	m_oCallback = new MPProxyPluginProducerVideoCallback(this);
	const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->setCallback(*m_oCallback);
}

MPProxyPluginProducerVideo::~MPProxyPluginProducerVideo()
{
	m_pcWrappedProducer = NULL;

	TSK_DEBUG_INFO("MPProxyPluginProducerVideo object destroyed");
}

int MPProxyPluginProducerVideo::prepareCallback(int nWidth, int nHeight, int nFps)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nFPS = nFps;

	// call parent
	return MPProxyPlugin::prepare();
}

int MPProxyPluginProducerVideo::startCallback()
{
		
	// call parent
	return MPProxyPlugin::start();
}

int MPProxyPluginProducerVideo::pauseCallback()
{

	// call parent
	return MPProxyPlugin::pause();
}

int MPProxyPluginProducerVideo::stopCallback()
{
	int ret;

	// call parent
	if((ret = MPProxyPlugin::stop())){
		TSK_DEBUG_ERROR("Failed to stop producer");
		return ret;
	}
	

	return ret;
}

int MPProxyPluginProducerVideo::push(const void* pcBuffer, unsigned nBufferSize, unsigned nFrameWidth, unsigned nFrameHeight)
{
	if(!m_bStarted){
		TSK_DEBUG_INFO("Video producer not started yet");
		return 0;
	}
	
	if(m_pcWrappedProducer && isValid()){
		if(m_nWidth != nFrameWidth || m_nHeight != nFrameHeight){
			if(!const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->setActualCameraOutputSize(nFrameWidth, nFrameHeight)){
				TSK_DEBUG_ERROR("Failed to change the camera output size (%u, %u)", nFrameWidth, nFrameHeight);
				return -1;
			}
			m_nWidth = nFrameWidth;
			m_nHeight = nFrameHeight;
		}
		return const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->push(pcBuffer, nBufferSize);
	}
		
	return 0;
}


} // namespace