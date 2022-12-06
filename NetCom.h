#pragma once
#include "CurlManager.h"
//#include "JsonTranser.h"
#define enum_to_string(val) #val
class NetCom final
{
public:
	static NetCom& instanse();
public:
	bool init();
	bool getMission(missionData& out_data);
	void sendState(const string& strUid, const int& stateType, string errMsg = "");
	void close();
public:
	enum stateType
	{
		error,
		finished,
		started
	};
private:
	NetCom() {};
	NetCom(NetCom&);
	NetCom& operator=(NetCom&) {};
private:
	CurlManager* _curlMana;
	//JsonTranser* _jsonTranser;
	missionData m_missionData;
};

