#include "CurlManager.h"
#include "common.h"
#include <iostream>
#include <sstream>
#include <cjson.h>
#include <comutil.h>
#include "LogManager.h"
#pragma comment(lib, "comsuppw.lib")//important
bool CurlManager::init(const string& ip, const string& port, const string& svrUri)
{
	m_ip = ip;
	m_port = port;
	m_svrUri = svrUri;
	m_ip_bind_port = m_ip + ":" + m_port;
	m_missionUri = m_svrUri + "/api/v2/jobHunting?ip=" + m_ip_bind_port;
	m_stateUri = m_svrUri + "/api/v2/jobsMsg";
	m_handle = curl_easy_init();
	if (m_handle == nullptr)
	{
		LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Failed to init curl!!!");
		return false;
	}
	return true;
}

size_t CurlManager::write_data(void* ptr, size_t size, size_t nmemb, void* stream)
{
	string data((const char*)ptr, (size_t)size * nmemb);
	*((stringstream*)stream) << data;
	return size * nmemb;
}

missionData CurlManager::parseMission()
{
	missionData data;
	cout << m_jsonStr << endl;
	//cout << GBKToUTF8(m_jsonStr) << endl;
	size_t pos = 0;
	while ((pos = m_jsonStr.find("\\u", pos)) != string::npos)
	{
		m_jsonStr.replace(pos, 2, "\\\\u");
		pos += 4;
	}
	cJSON* root = ::cJSON_Parse(m_jsonStr.c_str());
	char* r = cJSON_Print(root);
	string aaa = r;
	free(r);
	cout << "=================================================\n" << aaa << "========================" << endl;
	if (root != nullptr)
	{
		cJSON* dataItem = cJSON_GetObjectItem(root, "data");
		if (dataItem != nullptr)
		{
			data.uid = cJSON_GetObjectItem(dataItem, "uid")->valuestring;
			cJSON* contentItem = cJSON_GetObjectItem(dataItem, "jsonData");
			cJSON* templateItem = cJSON_GetObjectItem(contentItem, "template");
			data.contents.templateSrc = cJSON_GetObjectItem(templateItem, "src")->valuestring;
			data.contents.sessionid = cJSON_GetObjectItem(contentItem, "sessionid")->valuestring;
			cJSON* ossInfoItem = cJSON_GetObjectItem(contentItem, "ossinfo");
			data.contents.ossinfos.access_key_id = cJSON_GetObjectItem(ossInfoItem, "access_key_id")->valuestring;
			data.contents.ossinfos.access_key_secret = cJSON_GetObjectItem(ossInfoItem, "access_key_secret")->valuestring;
			data.contents.ossinfos.region = cJSON_GetObjectItem(ossInfoItem, "region")->valuestring;
			cJSON* assetsItem = cJSON_GetObjectItem(contentItem, "assets"); 
			int  array_size = cJSON_GetArraySize(assetsItem);
			for (int index = 0; index < array_size; ++index)
			{
				cJSON* pSub = cJSON_GetArrayItem(assetsItem, index);
				if (nullptr == pSub) { continue; }
				string type = cJSON_GetObjectItem(pSub, "type")->valuestring;
				if ("video" == type)
				{
					vedioInfo temp;
					temp.src = cJSON_GetObjectItem(pSub, "src")->valuestring;
					temp.layerName = cJSON_GetObjectItem(pSub, "layerName")->valuestring;
					temp.bucket = cJSON_GetObjectItem(pSub, "bucket")->valuestring;
					temp.type = type;
					data.contents.assets.vec_vidInfos.push_back(temp);
				}
				else if ("data" == type)
				{
					subInfo temp;
					temp.property = cJSON_GetObjectItem(pSub, "property")->valuestring;
					temp.layerName = cJSON_GetObjectItem(pSub, "layerName")->valuestring;
					temp.value = cJSON_GetObjectItem(pSub, "value")->valuestring;
					temp.type = type;
					data.contents.assets.assContent = temp;
				}
			}
			cJSON* postrenderLists = cJSON_GetObjectItem(cJSON_GetObjectItem(contentItem, "actions"), "postrender");
			array_size = cJSON_GetArraySize(postrenderLists);

			for (int index = 0; index < array_size; ++index)
			{
				cJSON* pSub = cJSON_GetArrayItem(postrenderLists, index);
				if (nullptr == pSub) { continue; }
				postrender temp;
				temp.output = cJSON_GetObjectItem(pSub, "output")->valuestring;
				temp.input = cJSON_GetObjectItem(pSub, "input")->valuestring;
				temp.bucket = cJSON_GetObjectItem(pSub, "bucket")->valuestring;
				data.contents.vec_postrenders.push_back(temp);
			}
		}
		cJSON_Delete(root);
	}
	return data;
}

bool CurlManager::getMission()
{
	if (m_handle != nullptr)
	{
		curl_easy_reset(m_handle);
		stringstream out;
		curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &out);
		curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(m_handle, CURLOPT_URL, m_missionUri.c_str());
		CURLcode res = curl_easy_perform(m_handle);
		if (res != CURLE_OK)
		{
			cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
			LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Can not get task, curl error! Try to reset curl!");
			close();
		}
		m_jsonStr = UTF8ToGBK(out.str());
		//cout << "UTF8ToGBK ============ " << m_jsonStr << endl;
		cJSON* root = ::cJSON_Parse(m_jsonStr.c_str());
		cJSON* codeItem = cJSON_GetObjectItem(root, "code");
		if (codeItem == nullptr || 1 == codeItem->valueint)
		{
			cJSON_Delete(root);
			return false;
		}
		cout << "[[[[[[[[[[[[[               " << out.str() << "          ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]" << endl;
		cJSON_Delete(root);
		return true;
	}
	if (!init(m_ip, m_port, m_svrUri))
		LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Failed to reset curl!!!");
	LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Curl handle is not init!");
	return false;
}

void CurlManager::sendState(const string& strUid, const string& strState, const string& strErrMsg)
{
	cJSON* postInfo = cJSON_CreateObject();
	if (postInfo == nullptr)
	{
		cout << "error : can not make state json contents!" << endl;
		return;
	}
	cJSON_AddStringToObject(postInfo, "uid", strUid.c_str());
	cJSON_AddStringToObject(postInfo, "state", strState.c_str());
	cJSON_AddStringToObject(postInfo, "msg", strErrMsg.c_str());
	char* r = cJSON_Print(postInfo);
	string str = r;
	free(r);

	curl_easy_reset(m_handle);
	stringstream out;
	curl_slist* headers = nullptr;
	
	headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(m_handle, CURLOPT_POST, 1);
	curl_easy_setopt(m_handle, CURLOPT_POSTFIELDS, str.c_str());

	curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &out);
	curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(m_handle, CURLOPT_URL, m_stateUri.c_str());

	CURLcode res = curl_easy_perform(m_handle);
	if (res != CURLE_OK)
	{
		cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
	}
	else
	{
		m_jsonStr = out.str();
		//cout << m_jsonStr << endl;
	}	
	cJSON_Delete(postInfo);
}

void CurlManager::close()
{
	curl_easy_cleanup(m_handle);
	m_handle = nullptr;
}
