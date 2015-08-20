Source code freely provided to you by <a href='http://www.doubango.org'> Doubango Telecom ®</a>.
<br />
This is part of <a href='http://code.google.com/p/sipml5/'>sipML5</a> solution and don't hesitate to test our [live demo](http://sipml5.org/).<br />



**webrtc2sip** is a smart and powerful gateway using [RTCWeb](http://en.wikipedia.org/wiki/WebRTC) and SIP to turn your browser into a phone with audio, video and SMS capabilities. The gateway allows your web browser to make and receive calls from/to any SIP-legacy network or PSTN.<br />
As an example, you will be able to make a call from your preferred web browser to a SIP-legacy softphone (e.g. <a href='http://www.counterpath.com/x-lite.html'>xlite</a>) or mobile/fixed phone.<br />
The gateway contains four modules: [SIP Proxy](#SIP_Proxy.md) | [RTCWeb Breaker](#RTCWeb_Breaker.md) | [Media Coder](#Media_Coder.md) | [Click-to-Call service](#Click-to-Call_service.md)..

![http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/architecture.png](http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/architecture.png) <br />
_Global architecture_


## SIP Proxy ##
The role of the SIP Proxy module is to convert the SIP transport from WebSocket protocol to <b>UDP</b>, <b>TCP</b> or <b>TLS</b> which are supported by all legacy networks. If your provider or hosted server supports SIP over <b>WebSocket</b> (e.g. Asterisk or Kamailio) then, you can bypass the module and connect the client directly to the endpoint.
Bypassing the SIP Proxy is not recommended if you’re planning to use the RTCWeb Breaker or Media Coder modules as this will requires maintaining two different connections.<br />
There are no special requirements for the end server to be able to talk to the Proxy module.

![http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/module_sipproxy.png](http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/module_sipproxy.png) <br />
_SIP Proxy architecture_


## RTCWeb Breaker ##
The RTCWeb specifications make support for **ICE** and **DTLS/SRTP** mandatory. The problem is that many SIP-legacy endpoints (e.g. PSTN network) do not support these features.
It’s up to the RTCWeb Breaker to negotiate and convert the media stream to allow these two worlds to interop. <br />
We highly recommend checking the [Technical Guide](http://webrtc2sip.org/technical-guide-1.0.pdf) to understand how to avoid **security issues** when using this module.
For example, **FreeSWITCH** do not support **ICE** which means it requires the RTCWeb Breaker in order to be able to connect the browser to a SIP-legacy endpoint.

![http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/module_rtcwebbreaker.png](http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/module_rtcwebbreaker.png) <br />
_RTCWeb Breaker architecture_


## Media Coder ##
The RTCWeb standard defined two MTI (Mandatory To Implement) audio codecs: <b>opus</b> and <b>g.711</b>.<br />
For now there are intense discussions about the MTI video codecs. The choice is between <b>VP8</b> and <b>H.264</b>. VP8 is royalty-free but not widely deployed while H.264 AVC is not free but widely deployed.
Google has decided to use <b>VP8</b> in Chrome while Ericsson uses <b>H.264 AVC</b> in [Bowser](https://labs.ericsson.com/apps/bowser). Mozilla and Opera Software will probably use VP8 and Microsoft H.264 AVC. As an example, the Media Coder will allow to make video calls between Chrome and Bowser.
Another example is calling a Telepresence system (e.g. Cisco) which most likely uses <b>H.264 SVC</b> from Chrome.

![http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/module_mediacoder.png](http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/module_mediacoder.png) <br />
_Media Coder architecture_

## Click-to-Call service ##
This is a complete SIP [click-to-call](http://en.wikipedia.org/wiki/Click-to-call) solution based on the three other components. The goal is to allow any person receiving your mails, visiting your website, reading your twitts, watching your Facebook/Google+ profile to call you on your mobile phone with a single click. As an example, click [here](http://click2dial.org/u/ZGlvcG1hbWFkb3VAZG91YmFuZ28ub3Jn) to call me on my mobile phone. <br />
For more information: http://click2dial.org

![http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/module_click-to-call.png](http://webrtc2sip.googlecode.com/svn/trunk/documentation/images/module_click-to-call.png) <br />
_Click-to-Call Components_

## Testing the gateway ##
Let's say the webrtc2sip gateway and SIP server are running on two different PCs with IP addresses equal to _192.168.0.1_ and _192.168.0.2_ respectively.

  1. Open [http://sipml5.org/expert.htm](http://sipml5.org/expert.htm) in your browser
  1. Fill _“WebSocket Server URL”_ field with the IP address and port where your webrtc2sip gateway is listening for incoming Websocket connections (e.g _ws://192.168.0.1:10060_ or _wss://192.168.0.1:10062_). **IMPORTANT:** Do not forget the url scheme (**ws://** or **wss://**).
  1. The _“SIP outbound Proxy URL”_ is used to set the destination IP address and Port to use for all outgoing requests regardless the domain name (a.k.a realm). This is a good option for developers using a SIP domain name without valid DNS A/NAPTR/SRV records. E.g. _udp://192.168.0.2:5060_.
  1. Check _“Enable RTCWeb Breaker”_ if you want to call a SIP-legacy endpoint.

## Security Issues ##
We highly recommend checking the [Technical Guide](http://webrtc2sip.org/technical-guide-1.0.pdf) to understand how to avoid **security issues** when using our gateway.

## Technical help ##
Please check our [issue tracker](http://code.google.com/p/webrtc2sip/issues/list), [developer group](https://groups.google.com/group/doubango) and [technical guide](http://webrtc2sip.org/technical-guide-1.0.pdf) if you have any problem. <br />

<br />
<br />
**© 2012-2013 Doubango Telecom** <br />
_Inspiring the future_