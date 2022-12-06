#include "NetCom.h"
#include <string>
#include <iostream>
#include "confmaner.h"
NetCom& NetCom::instanse()
{
	static NetCom NC;
	return NC;
}

bool NetCom::init()
{
	string svrIpPort = ConfRuner::instance().getKeyValue("taskSvr", "ip");
	svrIpPort = svrIpPort + ":" + ConfRuner::instance().getKeyValue("taskSvr", "port");

	_curlMana = new CurlManager;
	if (!_curlMana->init(ConfRuner::instance().getKeyValue("rendworker", "ip"), ConfRuner::instance().getKeyValue("rendworker", "port"), svrIpPort))
		return false;
	return true;
}

bool NetCom::getMission(missionData& out_data)
{
	if (!_curlMana->getMission())
		return false;
	m_missionData = _curlMana->parseMission();
	out_data = m_missionData;
	return true;
}

void NetCom::sendState(const string& strUid, const int& stateType, string errMsg)
{
	std::string strStateType;
	switch (stateType)
	{
	case error:
		strStateType = enum_to_string(error);
		break;
	case finished:
		strStateType = enum_to_string(finished);
		break;
	case started:
		strStateType = enum_to_string(started);
		break;
	default:
		std::cout << "Invalid state." << std::endl;
		return;
	}
	_curlMana->sendState(strUid, strStateType, errMsg);
}

void NetCom::close()
{
	if (_curlMana != nullptr)
	{
		_curlMana->close();
		delete _curlMana;
	}
}