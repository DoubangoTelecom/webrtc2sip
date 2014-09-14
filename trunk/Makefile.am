sbin_PROGRAMS = webrtc2sip

webrtc2sip_CPPFLAGS = \
${TINYSAK_CFLAGS} \
${TINYNET_CFLAGS} \
${TINYHTTP_CFLAGS} \
${TINYSIP_CFLAGS} \
${TINYDAV_CFLAGS} \
${TINYSDP_CFLAGS} \
${TINYBFCP_CFLAGS} \
${TINYSIGCOMP_CFLAGS} \
${TINYMEDIA_CFLAGS} \
${TINYXCAP_CFLAGS} \
${TINYSMS_CFLAGS} \
${TINYMSRP_CFLAGS} \
${TINYRTP_CFLAGS} \
${TINYIPSEC_CFLAGS} \
-I${LIBXML2_INCLUDE} \
-I${LIBPTHREAD_INCLUDE} \
-Itinywrap

webrtc2sip_LDADD = \
-L${LIBXML2_LIB} \
-L${LIBPTHREAD_LIB} \
${DOUBANGO_LIBS_FALLBACK} \
${TINYSAK_LIBS} \
${TINYNET_LIBS} \
${TINYHTTP_LIBS} \
${TINYSIP_LIBS} \
${TINYDAV_LIBS} \
${TINYSDP_LIBS} \
${TINYBFCP_LIBS} \
${TINYSIGCOMP_LIBS} \
${TINYMEDIA_LIBS} \
${TINYXCAP_LIBS} \
${TINYSMS_LIBS} \
${TINYMSRP_LIBS} \
${TINYRTP_LIBS} \
${TINYIPSEC_LIBS} \
-lxml2 \
-lpthread


webrtc2sip_SOURCES  = \
	mp_c2c.cc\
	mp_engine.cc\
	mp_mail.cc\
	mp_mediaproxy.cc\
	mp_mutex.cc\
	mp_net_transport.cc\
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
	db/sqlite/mp_db_sqlite.cc\
	db/sqlite/sqlite3.c\
	db/mp_db.cc\
	db/mp_db_model.cc

webrtc2sip_SOURCES  += \
	jsoncpp/jsoncpp.cc

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
	
