#pragma once
#include <string>
#include <map>
#include "singleton.h"
using std::string;
using std::map;
using std::multimap;
class IConfManer
{
public:
	virtual bool confLoad(std::string path) = 0;
	virtual bool confReload() = 0;
	virtual string getKeyValue(const string& sectionName, const string& key) = 0;
	virtual ~IConfManer() {}
	struct IConfData { virtual ~IConfData() {} };
};

struct keyVal
{
	string key;
	string value;
};

class ConfRuner final : public IConfManer, public SingleTon<ConfRuner>
{
public:
	ConfRuner();
	bool confLoad(std::string path) override;
	bool confReload() override;
	string getKeyValue(const string& sectionName, const string& key) override;
private:
	multimap<string, keyVal> m_multimap_contents;
	std::string m_path;
};