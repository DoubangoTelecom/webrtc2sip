sbin_PROGRAMS = webrtc2sip

webrtc2sip_CPPFLAGS = \
-I${DOUBANGO_INCLUDE}/tinysak\
-I${DOUBANGO_INCLUDE}/tinynet\
-I${DOUBANGO_INCLUDE}/tinyhttp\
-I${DOUBANGO_INCLUDE}/tinysip\
-I${DOUBANGO_INCLUDE}/tinydav\
-I${DOUBANGO_INCLUDE}/tinysdp\
-I${DOUBANGO_INCLUDE}/tinymedia\
-I${DOUBANGO_INCLUDE}/tinyxcap\
-I${DOUBANGO_INCLUDE}/tinysms\
-I${DOUBANGO_INCLUDE}/tinymsrp\
-I${DOUBANGO_INCLUDE}/tinyrtp\
-I${LIBXML2_INCLUDE}\
-I${LIBPTHREAD_INCLUDE}\
-Itinywrap

webrtc2sip_LDADD = \
-L${DOUBANGO_LIB}\
-L${LIBXML2_LIB}\
-L${LIBPTHREAD_INCLUDE}\
-ltinySAK\
-ltinySIP\
-ltinyNET\
-ltinyDAV\
-ltinyMEDIA\
-ltinyHTTP\
-ltinyXCAP\
-ltinySMS\
-ltinyMSRP\
-ltinySDP\
-ltinyRTP\
-lxml2\
-lpthread


webrtc2sip_SOURCES  = \
	mp_engine.cc\
	mp_mediaproxy.cc\
	mp_mutex.cc\
	mp_object.cc\
	mp_peer.cc\
	mp_proxyplugin.cc\
	mp_proxyplugin_consumer_audio.cc\
	mp_proxyplugin_consumer_video.cc\
	mp_proxyplugin_mgr.cc\
	mp_proxyplugin_producer_audio.cc\
	mp_proxyplugin_producer_video.cc\
	mp_session.cc\
	mp_session_av.cc\
	mp_wrap.cc

webrtc2sip_SOURCES  += \
	tinywrap/ActionConfig.cxx\
	tinywrap/AudioResampler.cxx\
	tinywrap/DDebug.cxx\
	tinywrap/MediaContent.cxx\
	tinywrap/MediaSessionMgr.cxx\
	tinywrap/Msrp.cxx\
	tinywrap/ProxyConsumer.cxx\
	tinywrap/ProxyPluginMgr.cxx\
	tinywrap/ProxyProducer.cxx\
	tinywrap/SafeObject.cxx\
	tinywrap/SipCallback.cxx\
	tinywrap/SipEvent.cxx\
	tinywrap/SipMessage.cxx\
	tinywrap/SipSession.cxx\
	tinywrap/SipStack.cxx\
	tinywrap/SipUri.cxx\
	tinywrap/SMSEncoder.cxx\
	tinywrap/Xcap.cxx
	
