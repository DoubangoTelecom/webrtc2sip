/* Copyright (C) 2012-2013 Doubango Telecom <http://www.doubango.org>
/* Copyright (C) 2012 Diop Mamadou Ibrahima
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

static char* sConfigXmlPath = NULL;
#define kSQLiteConnectionInfo  "./c2c_sqlite.db"
#define kSQLiteName "sqlite"
#define kMySQLConnectionInfo  NULL
#define kMySQLName "mysql"

#define mp_list_count(list)		tsk_list_count((list), tsk_null, tsk_null)
#define mp_str_is(str, val)		tsk_striequals((const char*)(str), val)
#define mp_str_is_star(str)		mp_str_is((str), "*")
#define mp_str_is_yes(str)		mp_str_is((str), "yes")
#define mp_strn_update(ppstr, newstr, n) if(ppstr){ if(*ppstr) free(*ppstr); *ppstr = tsk_strndup(newstr, n); }

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
							TSK_DEBUG_ERROR("Failed to set debug-level = %s\n", (const char*)pCurrNode->content);
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
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "enable-rtp-symetric")) // available since 2.1.0
					{
						TSK_DEBUG_INFO("enable-rtp-symetric = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setRtpSymetricEnabled(mp_str_is_yes(pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'enable-rtp-symetric': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "enable-100rel")) // available since 2.0.0
					{
						TSK_DEBUG_INFO("enable-100rel = %s", (const char*)pCurrNode->content);
						if(!(oEngine->set100relEnabled(mp_str_is_yes(pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'enable-100rel': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "enable-media-coder")) // available since 2.0.0
					{
						TSK_DEBUG_INFO("enable-media-coder = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setMediaCoderEnabled(mp_str_is_yes(pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'enable-media-coder': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "enable-videojb")) // available since 2.0.0
					{
						TSK_DEBUG_INFO("enable-videojb = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setVideoJbEnabled(mp_str_is_yes(pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'enable-videojb': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "video-size-pref")) // available since 2.1.0
					{
						const char* pcPrefVideoSize = (const char*)pCurrNode->content;
						TSK_DEBUG_INFO("video-size-pref = %s", pcPrefVideoSize);

						if(!oEngine->setPrefVideoSize(pcPrefVideoSize))
						{
							TSK_DEBUG_ERROR("Failed to set 'video-size-pref': %s", pcPrefVideoSize);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "rtp-buffsize")) // available since 2.0.0
					{
						TSK_DEBUG_INFO("rtp-buffsize = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setRtpBuffSize(atoi((const char*)pCurrNode->content))))
						{
							TSK_DEBUG_ERROR("Failed to set 'rtp-buffsize': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "avpf-tail-length")) // available since 2.0.0
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
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "srtp-mode")) // available since 2.0.0
					{
						TSK_DEBUG_INFO("srtp-mode = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setSRTPMode((const char*)pCurrNode->content)))
						{
							TSK_DEBUG_ERROR("Failed to set 'srtp-mode': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "srtp-type")) // available since 2.1.0
					{
						TSK_DEBUG_INFO("srtp-type = %s", (const char*)pCurrNode->content);
						if(!(oEngine->setSRTPType((const char*)pCurrNode->content)))
						{
							TSK_DEBUG_ERROR("Failed to set 'srtp-type': %s", (const char*)pCurrNode->content);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "ssl-certificates")) // available since 2.0.0
					{
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && mp_list_count(pParams) >= 3)
						{
							const char* pcPrivateKey = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcPublicKey = ((const tsk_param_t*)pParams->head->next->data)->name;
							const char* pcCA = ((const tsk_param_t*)pParams->head->next->next->data)->name;
							const char* pcVerify = pParams->head->next->next->next ? ((const tsk_param_t*)pParams->head->next->next->next->data)->name : "no"; // available since 2.1.0
							TSK_DEBUG_INFO("ssl-certificates = \n%s;\n%s;\n%s;\n%s", pcPrivateKey, pcPublicKey, pcCA, pcVerify);

							if(!oEngine->setSSLCertificates(mp_str_is_star(pcPrivateKey) ? NULL : pcPrivateKey, mp_str_is_star(pcPublicKey) ? NULL : pcPublicKey, mp_str_is_star(pcCA) ? NULL : pcCA, mp_str_is_yes(pcVerify)))
							{
								TSK_DEBUG_ERROR("Failed to set 'ssl-certificates': %s;\n%s;\n%s", pcPrivateKey, pcPublicKey, pcCA);
							}
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "codecs")) // available since 2.0.0
					{
						const char* pcCodecs = (const char*)pCurrNode->content;
						TSK_DEBUG_INFO("codecs = %s", pcCodecs);
						if(!oEngine->setCodecs(pcCodecs))
						{
							TSK_DEBUG_ERROR("Failed to set 'codecs': %s", pcCodecs);
						}
						break;
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "codec-opus-maxrates")) // available since 2.5.0
					{
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && mp_list_count(pParams) == 2)
						{
							const char* pcMaxPlaybackRate = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcMaxCaptureRate = ((const tsk_param_t*)pParams->head->next->data)->name;
							TSK_DEBUG_INFO("codec-opus-maxrates = %s;%s", pcMaxPlaybackRate, pcMaxCaptureRate);
							if(!oEngine->setCodecOpusMaxRates(atoi(pcMaxPlaybackRate), atoi(pcMaxCaptureRate)))
							{
								TSK_DEBUG_ERROR("Failed to set 'codec-opus-maxrates': %s;%s", pcMaxPlaybackRate, pcMaxCaptureRate);
							}
						}
						break;
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "stun-server")) // available since 2.5.1
					{
						size_t nCount;
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && (nCount = mp_list_count(pParams)) >= 2)
						{
							const char* pcServerIP = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcServerPort = ((const tsk_param_t*)pParams->head->next->data)->name;
							const char* pcUsrName = (nCount >= 3) ? ((const tsk_param_t*)pParams->head->next->next->data)->name : NULL;
							const char* pcUsrPwd = (nCount >= 4) ? ((const tsk_param_t*)pParams->head->next->next->next->data)->name : NULL;
							TSK_DEBUG_INFO("stun-server = %s;%s;-;-", pcServerIP, pcServerPort);
							if(!oEngine->setStunServer(pcServerIP, atoi(pcServerPort), mp_str_is_star(pcUsrName) ? NULL : pcUsrName, mp_str_is_star(pcUsrPwd) ? NULL : pcUsrPwd))
							{
								TSK_DEBUG_ERROR("Failed to set 'stun-server': %s;%s;-;-", pcServerIP, pcServerPort);
							}
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "enable-icestun")) // available since 2.5.1
					{
						const char* pcEnabled = (const char*)pCurrNode->content;
						TSK_DEBUG_INFO("enable-icestun = %s", pcEnabled);
						if(!oEngine->setIceStunEnabled(mp_str_is_yes(pcEnabled)))
						{
							TSK_DEBUG_ERROR("Failed to set 'enable-icestun': %s", pcEnabled);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "max-fds")) // available since 2.6.1
					{
						const char* pcMaxFds = (const char*)pCurrNode->content;
						TSK_DEBUG_INFO("max-fds = %s", pcMaxFds);
						if(!oEngine->setMaxFds(atoi(pcMaxFds)))
						{
							TSK_DEBUG_ERROR("Failed to set 'max-fds': %s", pcMaxFds);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "nameserver")) // available since 2.0.0
					{
						const char* pcDNSServer = (const char*)pCurrNode->content;
						TSK_DEBUG_INFO("nameserver = %s", pcDNSServer);
						if(!oEngine->addDNSServer(pcDNSServer))
						{
							TSK_DEBUG_ERROR("Failed to set 'nameserver': %s", pcDNSServer);
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "database")) // available since 2.3.0
					{
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && mp_list_count(pParams) >= 2)
						{
							const char* pcDbType = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcDbConnectionInfo = ((const tsk_param_t*)pParams->head->next->data)->name;
							TSK_DEBUG_INFO("database = %s;%s", pcDbType, pcDbConnectionInfo);

							if(mp_str_is_star(pcDbConnectionInfo))
							{
								if(tsk_striequals(kSQLiteName, pcDbType))
								{
									pcDbConnectionInfo = kSQLiteConnectionInfo;
								}
								else if(tsk_striequals(kMySQLName, pcDbType))
								{
									pcDbConnectionInfo = kMySQLConnectionInfo;
								}
							}

							if(!oEngine->setDbInfo(pcDbType, pcDbConnectionInfo))
							{
								TSK_DEBUG_ERROR("Failed to set 'database': %s", (const char*)pCurrNode->content);
							}
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "account-mail")) // available since 2.3.0
					{
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && mp_list_count(pParams) >= 7)
						{
							const char* pcSmtpScheme = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcLocalIP = ((const tsk_param_t*)pParams->head->next->data)->name;
							const char* pcLocalPort = ((const tsk_param_t*)pParams->head->next->next->data)->name;
							const char* pcSmtpHost = ((const tsk_param_t*)pParams->head->next->next->next->data)->name;
							const char* pcSmtpPort = ((const tsk_param_t*)pParams->head->next->next->next->next->data)->name;
							const char* pcEmail = ((const tsk_param_t*)pParams->head->next->next->next->next->next->data)->name;
							const char* pcAuthName = ((const tsk_param_t*)pParams->head->next->next->next->next->next->next->data)->name;
							const char* pcAuthPassword = ((const tsk_param_t*)pParams->head->next->next->next->next->next->next->next->data)->name;
							TSK_DEBUG_INFO("account-mail = %s;%s;%s;%s;%s;%s;password", pcSmtpScheme, pcLocalIP, pcLocalPort, pcSmtpHost, pcSmtpPort, pcAuthName);
							if(!oEngine->setMailAccountInfo(pcSmtpScheme, mp_str_is_star(pcLocalIP) ? NULL : pcLocalIP, mp_str_is_star(pcLocalPort) ? 0 : atoi(pcLocalPort), pcSmtpHost, atoi(pcSmtpPort), pcEmail, pcAuthName, pcAuthPassword))
							{
								TSK_DEBUG_ERROR("Failed to set 'account-mail': %s", (const char*)pCurrNode->content);
							}
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "account-sip-caller")) // available since 2.3.0
					{
						if((pParams = tsk_params_fromstring((const char*)pCurrNode->content, ";", tsk_true)) && mp_list_count(pParams) >= 5)
						{
							const char* pcDisplayName = ((const tsk_param_t*)pParams->head->data)->name;
							const char* pcImpu = ((const tsk_param_t*)pParams->head->next->data)->name;
							const char* pcImpi = ((const tsk_param_t*)pParams->head->next->next->data)->name;
							const char* pcRealm = ((const tsk_param_t*)pParams->head->next->next->next->data)->name;
							const char* pcPassword = ((const tsk_param_t*)pParams->head->next->next->next->next->data)->name;
							TSK_DEBUG_INFO("account-sip-caller = %s;%s;%s;%s;password", pcDisplayName, pcImpu, pcImpi, pcRealm);
							if(!oEngine->addAccountSipCaller(mp_str_is_star(pcDisplayName) ? NULL : pcDisplayName, pcImpu, pcImpi, pcRealm, pcPassword))
							{
								TSK_DEBUG_ERROR("Failed to add 'account-sip-caller': %s", (const char*)pCurrNode->content);
							}
						}
					}
					else if(pCurrNode->parent && tsk_striequals(pCurrNode->parent->name, "dtmf-type")) // available since 2.4.0
					{
						const char* pcDtmfType = (const char*)pCurrNode->content;
						TSK_DEBUG_INFO("dtmf-type = %s", pcDtmfType);
						if(!oEngine->setDtmfType(pcDtmfType))
						{
							TSK_DEBUG_ERROR("Failed to set 'dtmf-type': %s", pcDtmfType);
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
		TSK_DEBUG_ERROR("Failed to read xml config file at [%s]", xmlPath);
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

static int printUsage()
{
	fprintf(stdout, 
		"Usage: webrtc2sip [OPTION]\n\n"
		"--config=PATH     override the default path to the config.xml file\n"
		"--help            display this help and exit\n"
		"--version         output version information and exit\n"
		);

	return 0;
}

static int parseArgument(const char* arg, const char** name, tsk_size_t* name_size, const char** value, tsk_size_t* value_size)
{
	int32_t index, arg_size;
	if(tsk_strnullORempty(arg) || !name || !name_size || !value || !value_size){
		TSK_DEBUG_ERROR("Invalid parameter");
		return -1;
	}
	*name = *value = tsk_null;
	*name_size = *value_size = 0;
	arg_size = tsk_strlen(arg);

	*name = arg;
	index = tsk_strindexOf(arg, arg_size, "=");
	if(index <= 0){
		*name_size = arg_size;
		return 0;
	}
	*name_size = index;
	*value = &arg[index + 1];
	*value_size = (arg_size - index - 1);
	return 0;
}

static int parseArguments(int argc, char** argv)
{
	int i, ret;
	const char *name, *value;
	tsk_size_t name_size, value_size;

	if(argc <= 1 || !argv){
		return 0;
	}

	for(i = 1; i < argc; ++i){
		if((ret = parseArgument(argv[i], &name, &name_size, &value, &value_size))){
			printUsage();
			return ret;
		}
		if(tsk_strniequals("--config", name, name_size)){
			if(!value || !value_size){
				fprintf(stderr, "--config requires valid PATH\n");
				printUsage();
				exit(-1);
			}
			mp_strn_update(&sConfigXmlPath, value, value_size);
		}
		else if(tsk_strniequals("--help", name, name_size)){
			printUsage();
			exit(-1);
		}
		else if(tsk_strniequals("--version", name, name_size)){
			fprintf(stdout, "%d.%d.%d\n", WEBRTC2SIP_VERSION_MAJOR, WEBRTC2SIP_VERSION_MINOR, WEBRTC2SIP_VERSION_MICRO);
			exit(-1);
		}
		else{
			fprintf(stderr, "'%.*s' not valid as command argument\n", name_size, name);
			exit(-1);
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	bool bRet;
	int iRet, i = 0;
	char quit[4];

	printf("*******************************************************************\n"
		"Copyright (C) 2012-2013 Doubango Telecom <http://www.doubango.org>\n"
		"PRODUCT: webrtc2sip\n"
		"HOME PAGE: http://webrtc2sip.org\n"
		"LICENCE: GPLv3 or proprietary\n"
		"VERSION: %s\n"
		"'quit' to quit the application.\n"
		"*******************************************************************\n\n"
		, WEBRTC2SIP_VERSION_STRING);

	// parse command arguments
	if((iRet = parseArguments(argc, argv)) != 0){
		return iRet;
	}
	// create engine
	MPObjectWrapper<MPEngine*> oEngine = MPEngine::New();
	// parse "config.xml" file
	if((iRet = parseConfigRoot(oEngine, sConfigXmlPath)))
	{
		return iRet;
	}	
	if(!(bRet = oEngine->start()))
	{
		exit (-1);
	}
	
	while(true)
	{
		if((quit[i & 3] = getchar()) == 't')
		{
			if(quit[(i + 1) & 3] == 'q' && quit[(i + 2) & 3] == 'u' && quit[(i + 3) & 3] == 'i')
			{
				break;
			}
		}
		// FIXME: https://code.google.com/p/webrtc2sip/issues/detail?id=96
		tsk_thread_sleep(1);
		++i;
	}

	oEngine->stop();

	return 0;
}

