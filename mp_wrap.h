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
#if !defined(_MEDIAPROXY_CALLBACK_H_)
#define _MEDIAPROXY_CALLBACK_H_

#include "mp_config.h"
#include "mp_object.h"
#include "mp_peer.h"

#include "SipStack.h"

class RegistrationEvent;
class InviteEvent;
class DialogEvent;

namespace webrtc2sip {

class MPEngine;


// 
//	MPSipCallback
//
class MPSipCallback : public MPObject, public SipCallback
{
protected:
	MPSipCallback(MPObjectWrapper<MPEngine*> oEngine);
public:
	virtual ~MPSipCallback();
	virtual MP_INLINE const char* getObjectId() { return "Callback"; }
	virtual void attachMediaProxyPlugins(MPObjectWrapper<MPPeer*> oPeer);
	static MPObjectWrapper<MPSipCallback*> New(MPObjectWrapper<MPEngine*> oEngine);

	// SipCallback override
	virtual int OnRegistrationEvent(const RegistrationEvent* e);
	virtual int OnInviteEvent(const InviteEvent* e);
	virtual int OnDialogEvent(const DialogEvent* e) ;

private:
	MPObjectWrapper<MPEngine*> m_oEngine;
};


//
//	Stack
//
class MPSipStack : public MPObject
{
protected:
	MPSipStack(MPObjectWrapper<MPSipCallback*> oCallback, const char* pRealmUri, const char* pPrivateId, const char* pPublicId);
public:
	virtual ~MPSipStack();
	virtual MP_INLINE const char* getObjectId() { return "Stack"; }
	const SipStack* getWrappedStack();
	bool isValid();
	static MPObjectWrapper<MPSipStack*> New(MPObjectWrapper<MPSipCallback*> oCallback, const char* pRealmUri, const char* pPrivateId, const char* pPublicId);

private:
	SipStack* m_pStack;
};

} // namespace
#endif /* _MEDIAPROXY_CALLBACK_H_ */
