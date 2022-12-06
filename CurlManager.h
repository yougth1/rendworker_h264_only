#pragma once
#include <curl\\easy.h>
#include <curl\\multi.h>
#include <string>
#include "Agreenments.h"
using namespace std;
class CurlManager final
{
public:
	CurlManager() : m_handle(nullptr) {};
	bool init(const string& ip, const string& port, const string& svrUri);
	static size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream);
	bool getMission();
	void sendState(const string& strUid, const string& strState, const string& strErrMsg);
	missionData parseMission();
	void close();
private:
	string m_ip_bind_port;
	string m_svrUri;
	string m_ip;
	string m_port;
	string m_missionUri;
	string m_stateUri;
	string m_jsonStr;
	CURL* m_handle;
};

