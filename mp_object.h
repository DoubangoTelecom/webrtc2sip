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
#if !defined(_MEDIAPROXY_OBJECT_H_)
#define _MEDIAPROXY_OBJECT_H_

#include "mp_config.h"
#include "tsk_debug.h"

#define MPObjectSafeRelease(pMPObject)	(pMPObject) = NULL
#define MPObjectSafeFree				MPObjectSafeRelease

namespace webrtc2sip {

class MPObject
{
public:
	MPObject();
	MPObject(const MPObject &);
	virtual ~MPObject();

public:
	virtual MP_INLINE const char* getObjectId() = 0;
#if !defined(SWIG)
	MP_INLINE int GetRefCount() const{
		return m_nRefCount;
	}
	void operator=(const MPObject &);
#endif


public:
	int takeRef() const;
	int releaseRef() const;

private:
	mutable int	m_nRefCount;
};


//
//	MPObjectWrapper declaration
//
template<class MPObjectType>
class MPObjectWrapper{

public:
	MP_INLINE MPObjectWrapper(MPObjectType obj = NULL);
	MP_INLINE MPObjectWrapper(const MPObjectWrapper<MPObjectType> &obj);
	MP_INLINE virtual ~MPObjectWrapper();

#if !defined(SWIG)
public:
	MP_INLINE MPObjectWrapper<MPObjectType>& operator=(const MPObjectType other);
	MP_INLINE MPObjectWrapper<MPObjectType>& operator=(const MPObjectWrapper<MPObjectType> &other);
	MP_INLINE bool operator ==(const MPObjectWrapper<MPObjectType> other) const;
	MP_INLINE bool operator!=(const MPObjectWrapper<MPObjectType> &other) const;
	MP_INLINE bool operator <(const MPObjectWrapper<MPObjectType> other) const;
	MP_INLINE MPObjectType operator->() const;
	MP_INLINE MPObjectType operator*() const;
	MP_INLINE operator bool() const;
#endif

protected:
	MP_INLINE int takeRef();
	MP_INLINE int releaseRef();

	MP_INLINE MPObjectType GetWrappedMPObject() const;
	MP_INLINE void WrapMPObject(MPObjectType obj);

private:
	MPObjectType m_WrappedMPObject;
};

//
//	MPObjectWrapper implementation
//
template<class MPObjectType>
MPObjectWrapper<MPObjectType>::MPObjectWrapper(MPObjectType obj) { 
	WrapMPObject(obj), takeRef();
}

template<class MPObjectType>
MPObjectWrapper<MPObjectType>::MPObjectWrapper(const MPObjectWrapper<MPObjectType> &obj) {
	WrapMPObject(obj.GetWrappedMPObject()),
	takeRef();
}

template<class MPObjectType>
MPObjectWrapper<MPObjectType>::~MPObjectWrapper(){
	releaseRef(),
	WrapMPObject(NULL);
}


template<class MPObjectType>
int MPObjectWrapper<MPObjectType>::takeRef(){
	if(m_WrappedMPObject /*&& m_WrappedMPObject->getRefCount() At startup*/){
		return m_WrappedMPObject->takeRef();
	}
	return 0;
}

template<class MPObjectType>
int MPObjectWrapper<MPObjectType>::releaseRef() {
	if(m_WrappedMPObject && m_WrappedMPObject->GetRefCount()){
		if(m_WrappedMPObject->releaseRef() == 0){
			delete m_WrappedMPObject, m_WrappedMPObject = NULL;
		}
		else{
			return m_WrappedMPObject->GetRefCount();
		}
	}
	return 0;
}

template<class MPObjectType>
MPObjectType MPObjectWrapper<MPObjectType>::GetWrappedMPObject() const{
	return m_WrappedMPObject;
}

template<class MPObjectType>
void MPObjectWrapper<MPObjectType>::WrapMPObject(const MPObjectType obj){
	if(obj){
		if(!(m_WrappedMPObject = dynamic_cast<MPObjectType>(obj))){
			TSK_DEBUG_ERROR("Trying to wrap an object with an invalid type");
		}
	}
	else{
		m_WrappedMPObject = NULL;
	}
}

template<class MPObjectType>
MPObjectWrapper<MPObjectType>& MPObjectWrapper<MPObjectType>::operator=(const MPObjectType obj){
	releaseRef();
	WrapMPObject(obj), takeRef();
	return *this;
}

template<class MPObjectType>
MPObjectWrapper<MPObjectType>& MPObjectWrapper<MPObjectType>::operator=(const MPObjectWrapper<MPObjectType> &obj){
	releaseRef();
	WrapMPObject(obj.GetWrappedMPObject()), takeRef();
	return *this;
}


template<class MPObjectType>
bool MPObjectWrapper<MPObjectType>::operator ==(const MPObjectWrapper<MPObjectType> other) const {
	return GetWrappedMPObject() == other.GetWrappedMPObject();
}

template<class MPObjectType>
bool MPObjectWrapper<MPObjectType>::operator!=(const MPObjectWrapper<MPObjectType> &other) const {
	return GetWrappedMPObject() != other.GetWrappedMPObject();
}

template<class MPObjectType>
bool MPObjectWrapper<MPObjectType>::operator <(const MPObjectWrapper<MPObjectType> other) const {
	return GetWrappedMPObject() < other.GetWrappedMPObject();
}

template<class MPObjectType>
MPObjectWrapper<MPObjectType>::operator bool() const {
	return (GetWrappedMPObject() != NULL);
}

template<class MPObjectType>
MPObjectType MPObjectWrapper<MPObjectType>::operator->() const {
	return GetWrappedMPObject();
}

template<class MPObjectType>
MPObjectType MPObjectWrapper<MPObjectType>::operator*() const{
	return GetWrappedMPObject();
}

} // namespace
#endif /* _MEDIAPROXY_OBJECT_H_ */
