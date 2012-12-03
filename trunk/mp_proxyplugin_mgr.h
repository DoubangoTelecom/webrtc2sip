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
#if !defined(_MEDIAPROXY_PROXYPLUGIN_MGR_H_)
#define _MEDIAPROXY_PROXYPLUGIN_MGR_H_

#include "mp_config.h"
#include "mp_proxyplugin.h"

#include "tsk_mutex.h"

#include <map>

namespace webrtc2sip {

class MPProxyPluginMgrCallback;
typedef std::map<uint64_t, MPObjectWrapper<MPProxyPlugin*> > MPMapOfPlugins;

//
//	MPProxyPluginMgr
//
class MPProxyPluginMgr
{
private:
	static ProxyPluginMgr* g_pPluginMgr;
	static MPProxyPluginMgrCallback* g_pPluginMgrCallback;
	static MPMapOfPlugins* g_pPlugins;
	static tsk_mutex_handle_t* g_phMutex;
	static bool g_bInitialized;

public:
	static void initialize();
	static void deInitialize();
	
	static inline MPObjectWrapper<MPProxyPlugin*> findPlugin(uint64_t nId){
		MPMapOfPlugins::iterator it = MPProxyPluginMgr::g_pPlugins->find(nId);
		MPObjectWrapper<MPProxyPlugin*> pPlugin = NULL;
		if(it != MPProxyPluginMgr::g_pPlugins->end()){
			pPlugin = it->second;
		}
		return pPlugin;
	}

	static inline void erasePlugin(uint64_t nId){
		MPMapOfPlugins::iterator it;
		if((it = MPProxyPluginMgr::g_pPlugins->find(nId)) != MPProxyPluginMgr::g_pPlugins->end()){
			MPObjectWrapper<MPProxyPlugin*> pPlugin = it->second;
			MPProxyPluginMgr::g_pPlugins->erase(it);
			pPlugin->invalidate();
		}
	}

	static inline MPMapOfPlugins* getPlugins(){
		return MPProxyPluginMgr::g_pPlugins;
	}
	
	static inline tsk_mutex_handle_t* getMutex(){
		return MPProxyPluginMgr::g_phMutex;
	}

	static inline ProxyPluginMgr* getPluginMgr(){
		return const_cast<ProxyPluginMgr* > (MPProxyPluginMgr::g_pPluginMgr);
	}
};

//
//	MPProxyPluginMgrCallback
//
class MPProxyPluginMgrCallback : public ProxyPluginMgrCallback
{
public:
	MPProxyPluginMgrCallback();
	virtual ~MPProxyPluginMgrCallback();

public: /* override */
	virtual int OnPluginCreated(uint64_t id, enum twrap_proxy_plugin_type_e type);
	virtual int OnPluginDestroyed(uint64_t id, enum twrap_proxy_plugin_type_e type);
};

} // namespace
#endif /* _MEDIAPROXY_PROXYPLUGIN_MGR_H_ */
