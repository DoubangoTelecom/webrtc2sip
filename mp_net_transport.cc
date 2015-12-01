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
#include "mp_net_transport.h"

#include "tsk_string.h"
#include "tsk_memory.h"
#include "tsk_debug.h"

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
, m_bIPv6(false)
, m_pWrappedPeersMutex(NULL)
, m_pAllowedRemoteHost(NULL)
{
	m_eType = eType;
	const char *pcDescription;
	tnet_socket_type_t eSocketType;
	
	if(pcLocalIP && nLocalPort)
	{
		m_bIPv6 = (tnet_get_family(pcLocalIP, nLocalPort) == AF_INET6);
	}

	switch(eType)
	{
		case MPNetTransporType_TCP:
			{
				pcDescription = m_bIPv6 ? "TCP/IPv6 transport" : "TCP/IPv4 transport";
				eSocketType = m_bIPv6 ? tnet_socket_type_tcp_ipv6 : tnet_socket_type_tcp_ipv4;
				break;
			}
		case MPNetTransporType_TLS:
			{
				pcDescription = m_bIPv6 ? "TLS/IPv6 transport" : "TLS/IPv4 transport";
				eSocketType = m_bIPv6 ? tnet_socket_type_tls_ipv6 : tnet_socket_type_tls_ipv4;
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
	TSK_FREE(m_pAllowedRemoteHost);
}

bool MPNetTransport::setAllowedRemoteHost(const char* pcAllowedRemoteHost)
{
	tsk_strupdate(&m_pAllowedRemoteHost, pcAllowedRemoteHost);
	return true;
}

bool MPNetTransport::setSSLCertificates(const char* pcPrivateKey, const char* pcPublicKey, const char* pcCA, bool bVerify /*= false*/)
{
	return (tnet_transport_tls_set_certs(m_pWrappedTransport, pcCA, pcPublicKey, pcPrivateKey, (bVerify ? tsk_true : tsk_false)) == 0);
}

bool MPNetTransport::start()
{
	m_bStarted = (tnet_transport_start(m_pWrappedTransport) == 0);
	if (m_bStarted) {
		// Update IPv6 information now that the transport is started
		m_bIPv6 = TNET_SOCKET_TYPE_IS_IPV6(((const tnet_transport_t*)m_pWrappedTransport)->master->type);
	}
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

bool MPNetTransport::havePeer(MPNetFd nFd)
{
	MPObjectWrapper<MPNetPeer*> oPeer = getPeerByFd(nFd);
	return !!oPeer;
}

int MPNetTransport::MPNetTransportCb_Stream(const tnet_transport_event_t* e)
{
	MPObjectWrapper<MPNetPeer*> oPeer = NULL;
	MPNetTransport* This = (MPNetTransport*)e->callback_data;

	switch(e->type)
	{	
		case event_closed:
		case event_removed:
		case event_error:
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
				TSK_DEBUG_INFO("New incoming connection. AllowedRemoteHost = %s", This->m_pAllowedRemoteHost);
				if (!tsk_strnullORempty(This->m_pAllowedRemoteHost))
				{
#define ERROR_MSG_INVALID_DOMAIN "HTTP/1.1 403 Invalid domain\r\n" \
		"Server: webrtc2sip Media Server " WEBRTC2SIP_VERSION_STRING "\r\n" \
		"Access-Control-Allow-Origin: *\r\n" \
		"Content-Length: %u\r\n" \
		"Connection: Close\r\n" \
		"\r\n"
					tnet_ip_t ip_remote, ip_domain;
					struct sockaddr_storage addr_domain;
					int ret;
					tnet_fd_t local_fd = e->local_fd;
					static const char _error_msg_invalid_domain_ptr[] = ERROR_MSG_INVALID_DOMAIN;
					static const size_t _error_msg_invalid_domain_size = sizeof(_error_msg_invalid_domain_ptr);
					if ((ret = tnet_get_peerip(local_fd, &ip_remote)) != 0)
					{
						This->sendData(local_fd, _error_msg_invalid_domain_ptr, _error_msg_invalid_domain_size);
						TSK_DEBUG_ERROR("tnet_get_peerip(%d) failed", local_fd);
						tnet_transport_remove_socket(This->m_pWrappedTransport, &local_fd);
						return ret;
					}
					// Update domain each time because it could change (DNS)
					if ((ret = tnet_sockaddr_init(This->m_pAllowedRemoteHost, 80, ((const tnet_transport_t*)This->m_pWrappedTransport)->master->type, &addr_domain)) != 0)
					{
						This->sendData(local_fd, _error_msg_invalid_domain_ptr, _error_msg_invalid_domain_size);
						TSK_DEBUG_ERROR("tnet_sockaddr_init(%s:%d) failed", This->m_pAllowedRemoteHost, 80);
						tnet_transport_remove_socket(This->m_pWrappedTransport, &local_fd);
						return ret;
					}
					if ((ret = tnet_get_sockip((const struct sockaddr*)&addr_domain, &ip_domain)) != 0)
					{
						This->sendData(local_fd, _error_msg_invalid_domain_ptr, _error_msg_invalid_domain_size);
						TSK_DEBUG_ERROR("tnet_get_sockip() failed: %d", ret);
						tnet_transport_remove_socket(This->m_pWrappedTransport, &local_fd);
						return ret;
					}
					if (!tsk_striequals(ip_remote, ip_domain))
					{
						This->sendData(local_fd, _error_msg_invalid_domain_ptr, _error_msg_invalid_domain_size);
						TSK_DEBUG_ERROR("Invalid domain name: %s<>%s", ip_remote, ip_domain);
						tnet_transport_remove_socket(This->m_pWrappedTransport, &local_fd);
						return -1;
					}
				}
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
		default:
			{
				break;
			}
	}

	return 0;
}


}//namespace