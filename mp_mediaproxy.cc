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
#include "mp_engine.h"

#include <libxml/tree.h>

#define kConfigXmlPath			NULL

#define mp_list_count(list)		tsk_list_count((list), tsk_null, tsk_null)
#define mp_str_is(str, val)		tsk_striequals((const char*)(str), val)
#define mp_str_is_star(str)		mp_str_is((str), "*")
#define mp_str_is_yes(str)		mp_str_is((str), "yes")

using namespace webrtc2sip;

static int parseConfigNode(xmlNode *pNode, MPObjectWrapper<MPEngine*> oEngine)
{
	xmlNode *pCurrNode;
	int iRet;
	tsk_params_L_t* pParams = tsk_null;

	if(!pNode || !oEngine)
	{
		TSK_DEBUG_ERROR("Invalid argument");
		return (-1);
	}

	for(pCurrNode = pNode; pCurrNode; pCurrNode = pCurrNode->next)
	{
		switch(pCurrNode->type)
		{
			case XML_ELEMENT_NODE:
				{
					break;
				}
			case XML_TEXT_NODE:
				{
					if(!pCurrNode->content)
					{
						break;
					}

					if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "debug-level"))
					{
						TSK_DEBUG_INFO("debug-level = %s\n", (const char*)pCurrNode->content);
						if(!oEngine->setDebugLevel((const char*)pCurrNode->content)){
							TSK_DEBUG_INFO("Failed to set debug-level = %s\n", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "transport"))
					{
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && mp_list_count(pParams) == 3)
						{
							const char* pcProto = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcLocalIP = ((const tsk_param_t*)pParams->head->next->data)->name;
							const char* pcLocalPort = ((const tsk_param_t*)pParams->head->next->next->data)->name;
							TSK_DEBUG_INFO("transport = %s://%s:%s", pcProto, pcLocalIP, pcLocalPort);

							if(!oEngine->addTransport(pcProto, mp_str_is_star(pcLocalPort) ? TNET_SOCKET_PORT_ANY : atoi(pcLocalPort), mp_str_is_star(pcLocalIP) ? TNET_SOCKET_HOST_ANY : pcLocalIP))
							{
								TSK_DEBUG_ERROR("Failed to add 'transport': %s://%s:%s", pcLocalPort, pcLocalIP, pcLocalPort);
							}
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "enable-100rel"))
					{
						TSK_DEBUG_INFO("enable-100rel = %s", (const char*)pCurrNode->content);
						if(!(oEngine->set100relEnabled(mp_str_is_yes(pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'enable-100rel': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "enable-media-coder"))
					{
						TSK_DEBUG_INFO("enable-media-coder = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setMediaCoderEnabled(mp_str_is_yes(pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'enable-media-coder': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "enable-videojb"))
					{
						TSK_DEBUG_INFO("enable-videojb = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setVideoJbEnabled(mp_str_is_yes(pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'enable-videojb': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "rtp-buffsize"))
					{
						TSK_DEBUG_INFO("rtp-buffsize = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setRtpBuffSize(atoi((const char*)pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'rtp-buffsize': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "avpf-tail-length"))
					{
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && mp_list_count(pParams) == 2)
						{
							const char* pcMin = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcMax = ((const tsk_param_t*)pParams->head->next->data)->name;
							TSK_DEBUG_INFO("avpf-tail-length = [%s-%s]", pcMin, pcMax);

							if(!oEngine->setAvpfTail(atoi(pcMin), atoi(pcMax)))
							{
								TSK_DEBUG_ERROR("Failed to set 'avpf-tail-length': [%s-%s]", pcMin, pcMax);
							}
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "ssl-certificates"))
					{
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && mp_list_count(pParams) == 3)
						{
							const char* pcPrivateKey = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcPublicKey = ((const tsk_param_t*)pParams->head->next->data)->name;
							const char* pcCA = ((const tsk_param_t*)pParams->head->next->next->data)->name;
							TSK_DEBUG_INFO("ssl-certificates = %s;\n%s;\n%s", pcPrivateKey, pcPublicKey, pcCA);

							if(!oEngine->setSSLCertificate(mp_str_is_star(pcPrivateKey) ? NULL : pcPrivateKey, mp_str_is_star(pcPublicKey) ? NULL : pcPublicKey, mp_str_is_star(pcCA) ? NULL : pcCA))
							{
								TSK_DEBUG_ERROR("Failed to set 'ssl-certificates': %s;\n%s;\n%s", pcPrivateKey, pcPublicKey, pcCA);
							}
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "codecs"))
					{
						int i;
						struct codec{ const char* name; tmedia_codec_id_t id; };
						static const codec aCodecNames[] = { 
							{"pcma", tmedia_codec_id_pcma}, 
							{"pcmu", tmedia_codec_id_pcmu}, 
							{"amr-nb-be", tmedia_codec_id_amr_nb_be},
							{"amr-nb-oa", tmedia_codec_id_amr_nb_oa},
							{"speex-nb", tmedia_codec_id_speex_nb}, 
							{"speex-wb", tmedia_codec_id_speex_wb}, 
							{"speex-uwb", tmedia_codec_id_speex_uwb}, 
							{"g729", tmedia_codec_id_g729ab}, 
							{"gsm", tmedia_codec_id_gsm}, 
							{"g722", tmedia_codec_id_g722}, 
							{"ilbc", tmedia_codec_id_ilbc},
							{"h264-bp", tmedia_codec_id_h264_bp}, 
							{"h264-mp", tmedia_codec_id_h264_mp}, 
							{"vp8", tmedia_codec_id_vp8}, 
							{"h263", tmedia_codec_id_h263}, 
							{"h263+", tmedia_codec_id_h263p}, 
							{"theora", tmedia_codec_id_theora}, 
							{"mp4v-es", tmedia_codec_id_mp4ves_es} 
						};
						static const int nCodecsCount = sizeof(aCodecNames) / sizeof(aCodecNames[0]);
					
						int64_t nCodecs = (int64_t)tmedia_codec_id_none;
						
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)))
						{
							const tsk_list_item_t* item;
							tsk_list_foreach(item, pParams)
							{	
								const char* pcCodecName = ((const tsk_param_t*)item->data)->name;
								for(i = 0; i < nCodecsCount; ++i)
								{
									if(tsk_striequals(aCodecNames[i].name, pcCodecName))
									{
										nCodecs |= (int64_t)aCodecNames[i].id;
										break;
									}
								}
							}
						}
						TSK_DEBUG_INFO("codecs = %lld: %s", nCodecs, (const char*)pCurrNode->content);
						oEngine->setCodecs(nCodecs);
						break;
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "nameserver"))
					{
						const char* pcDNSServer = (const char*)pCurrNode->content;
						TSK_DEBUG_INFO("nameserver = %s", pcDNSServer);

						if(!oEngine->addDNSServer(pcDNSServer))
						{
							TSK_DEBUG_ERROR("Failed to set 'nameserver': %s", pcDNSServer);
						}
					}


				break;
			}
		}
		
		TSK_OBJECT_SAFE_FREE(pParams);

        if(pCurrNode->children && (iRet = parseConfigNode(pCurrNode->children, oEngine)))
		{
			return (iRet);
		}
    }

	return (0);
}

static int parseConfigRoot(MPObjectWrapper<MPEngine*> oEngine, const char* xmlPath = NULL)
{
	xmlDoc *pDoc;
    xmlNode *pRootElement;

	if(!oEngine)
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return (-1);
	}
	if(!xmlPath)
	{
		xmlPath = "./config.xml";
	}

	if(!(pDoc = xmlReadFile(xmlPath, NULL, 0)))
	{
		TSK_DEBUG_ERROR("Failed to read xml config file at %s", xmlPath);
		return (-2);
	}

	if(!(pRootElement = xmlDocGetRootElement(pDoc)))
	{
		TSK_DEBUG_ERROR("Failed to get root element for xml config file at %s", xmlPath);
		xmlFreeDoc(pDoc);
		return (-3);
	}

	int iRet = parseConfigNode(pRootElement, oEngine);

	xmlCleanupParser();

	return (iRet);
}

int main(int argc, char** argv)
{
	bool bRet;
	int iRet, i = 0;
	char quit[4];

	printf("Copyright (C) 2012 Doubango Telecom <http://www.doubango.org>\n\n");

	MPObjectWrapper<MPEngine*> oEngine = MPEngine::New();
	if((iRet = parseConfigRoot(oEngine, kConfigXmlPath)))
	{
		return iRet;
	}	
	if(!(bRet = oEngine->start()))
	{
		exit (-1);
	}
	
	while(true)
	{
		if((quit[i & 3] = getchar()) == 't'){
			if(quit[(i + 1) & 3] == 'q' && quit[(i + 2) & 3] == 'u' && quit[(i + 3) & 3] == 'i'){
				break;
			}
		}
		
		++i;
	}

	return 0;
}

