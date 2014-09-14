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
#include "mp_net_transport.h"

#include <assert.h>

#if !defined(kMPMaxStreamBufferSize)
#	define kMPMaxStreamBufferSize 0xFFFF
#endif

namespace webrtc2sip {

//
//	MPNetPeer
//

// IMPORTANT: data sent using this function will never be encrypted
bool MPNetPeer::sendData(const void* pcDataPtr, size_t nDataSize)
{
	if(!pcDataPtr || !nDataSize)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}
	return (tnet_sockfd_send(getFd(), pcDataPtr, nDataSize, 0) == nDataSize);
}

//
//	MPNetTransport
//
MPNetTransport::MPNetTransport(MPNetTransporType_t eType, const char* pcLocalIP, unsigned short nLocalPort)
: m_bValid(false)
, m_bStarted(false)
, m_pWrappedPeersMutex(NULL)
{
	m_eType = eType;
	const char *pcDescription;
	tnet_socket_type_t eSocketType;
	bool bIsIPv6 = false;
	
	if(pcLocalIP && nLocalPort)
	{
		bIsIPv6 = (tnet_get_family(pcLocalIP, nLocalPort) == AF_INET6);
	}

	switch(eType)
	{
		case MPNetTransporType_TCP:
			{
				pcDescription = bIsIPv6 ? "TCP/IPv6 transport" : "TCP/IPv4 transport";
				eSocketType = bIsIPv6 ? tnet_socket_type_tcp_ipv6 : tnet_socket_type_tcp_ipv4;
				break;
			}
		case MPNetTransporType_TLS:
			{
				pcDescription = bIsIPv6 ? "TLS/IPv6 transport" : "TLS/IPv4 transport";
				eSocketType = bIsIPv6 ? tnet_socket_type_tls_ipv6 : tnet_socket_type_tls_ipv4;
				break;
			}
		default:
			{
				assert(false);
				break;
			}
	}

	if((m_pWrappedTransport = tnet_transport_create(pcLocalIP, nLocalPort, eSocketType, pcDescription)))
	{
		if(TNET_SOCKET_TYPE_IS_STREAM(eSocketType))
		{
			tnet_transport_set_callback(m_pWrappedTransport, MPNetTransport::MPNetTransportCb_Stream, this);
		}
		else
		{
			assert(false);
		}
	}

	if((m_pWrappedPeersMutex = tsk_mutex_create()))
	{
		m_bValid = true;
	}
}

MPNetTransport::~MPNetTransport()
{
	stop();
	TSK_OBJECT_SAFE_FREE(m_pWrappedTransport);
	if(m_pWrappedPeersMutex)
	{
		tsk_mutex_destroy(&m_pWrappedPeersMutex);
	}
}

bool MPNetTransport::setSSLCertificates(const char* pcPrivateKey, const char* pcPublicKey, const char* pcCA, bool bVerify /*= false*/)
{
	return (tnet_transport_tls_set_certs(m_pWrappedTransport, pcCA, pcPublicKey, pcPrivateKey, (bVerify ? tsk_true : tsk_false)) == 0);
}

bool MPNetTransport::start()
{
	m_bStarted = (tnet_transport_start(m_pWrappedTransport) == 0);
	return m_bStarted;
}

MPNetFd MPNetTransport::connectTo(const char* pcHost, unsigned short nPort)
{
	if(!pcHost || !nPort)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return TNET_INVALID_FD;
	}
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Transport not valid");
		return TNET_INVALID_FD;
	}
	if(!isStarted())
	{
		TSK_DEBUG_ERROR("Transport not started");
		return TNET_INVALID_FD;
	}

	return tnet_transport_connectto_2(m_pWrappedTransport, pcHost, nPort);
}

bool MPNetTransport::isConnected(MPNetFd nFd)
{
	MPObjectWrapper<MPNetPeer*> oPeer = getPeerByFd(nFd);
	return (oPeer && oPeer->isConnected());
}

bool MPNetTransport::sendData(MPNetFd nFdFrom, const void* pcDataPtr, size_t nDataSize)
{
	if(!pcDataPtr || !nDataSize || !MPNetFd_IsValid(nFdFrom))
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}
	return (tnet_transport_send(m_pWrappedTransport, nFdFrom, pcDataPtr, nDataSize) == nDataSize);
}

bool MPNetTransport::stop()
{
	m_bStarted = false;
	return (tnet_transport_shutdown(m_pWrappedTransport) == 0);
}

MPObjectWrapper<MPNetPeer*> MPNetTransport::getPeerByFd(MPNetFd nFd)
{
	tsk_mutex_lock(m_pWrappedPeersMutex);
	MPObjectWrapper<MPNetPeer*> m_Peer = NULL;

	std::map<MPNetFd, MPObjectWrapper<MPNetPeer*> >::iterator iter = m_Peers.find(nFd);
	if(iter != m_Peers.end()){
		m_Peer = iter->second;
	}
	tsk_mutex_unlock(m_pWrappedPeersMutex);

	return m_Peer;
}

void MPNetTransport::insertPeer(MPObjectWrapper<MPNetPeer*> oPeer)
{
	tsk_mutex_lock(m_pWrappedPeersMutex);
	if(oPeer){
		m_Peers.insert( std::pair<MPNetFd, MPObjectWrapper<MPNetPeer*> >(oPeer->getFd(), oPeer) );
	}
	tsk_mutex_unlock(m_pWrappedPeersMutex);
}

void MPNetTransport::removePeer(MPNetFd nFd)
{
	tsk_mutex_lock(m_pWrappedPeersMutex);
	std::map<MPNetFd, MPObjectWrapper<MPNetPeer*> >::iterator iter;
	if((iter = m_Peers.find(nFd)) != m_Peers.end()){
		MPObjectWrapper<MPNetPeer*> oPeer = iter->second;
		m_Peers.erase(iter);
	}
	tsk_mutex_unlock(m_pWrappedPeersMutex);
}

int MPNetTransport::MPNetTransportCb_Stream(const tnet_transport_event_t* e)
{
	MPObjectWrapper<MPNetPeer*> oPeer = NULL;
	MPNetTransport* This = (MPNetTransport*)e->callback_data;

	switch(e->type)
	{	
		case event_closed:
		case event_removed:
			{
				oPeer = (This)->getPeerByFd(e->local_fd);
				if(oPeer)
				{
					oPeer->setConnected(false);
					(This)->removePeer(e->local_fd);
					if((This)->m_oCallback)
					{
						(This)->m_oCallback->onConnectionStateChanged(oPeer);
					}
				}
				break;
			}
		case event_connected:
		case event_accepted:
			{
				oPeer = (This)->getPeerByFd(e->local_fd);
				if(oPeer)
				{
					oPeer->setConnected(true);
				}
				else
				{
					oPeer = new MPNetPeerStream(e->local_fd, true);
					(This)->insertPeer(oPeer);
				}
				if((This)->m_oCallback)
				{
					(This)->m_oCallback->onConnectionStateChanged(oPeer);
				}
				break;
			}

		case event_data:
			{
				oPeer = (This)->getPeerByFd(e->local_fd);
				if(!oPeer)
				{
					TSK_DEBUG_ERROR("Data event but no peer found!");
					return -1;
				}
				
				size_t nConsumedBytes = oPeer->getDataSize();
				if((nConsumedBytes + e->size) > kMPMaxStreamBufferSize)
				{
					TSK_DEBUG_ERROR("Stream buffer too large[%u > %u]. Did you forget to consume the bytes?", (nConsumedBytes + e->size), kMPMaxStreamBufferSize);
					dynamic_cast<MPNetPeerStream*>(*oPeer)->cleanupData();
				}
				else
				{
					if((This)->m_oCallback)
					{
						if(dynamic_cast<MPNetPeerStream*>(*oPeer)->appenData(e->data, e->size))
						{
							nConsumedBytes += e->size;
						}
						(This)->m_oCallback->onData(oPeer, nConsumedBytes);
					}
					if(nConsumedBytes)
					{
						dynamic_cast<MPNetPeerStream*>(*oPeer)->remoteData(0, nConsumedBytes);
					}
				}
				break;
			}

		case event_error:
		default:
			{
				break;
			}
	}

	return 0;
}


}//namespace