#include "confmaner.h"
#include <fstream>
#include <iostream>
#include "SimpleIni.h"
using std::list;

ConfRuner::ConfRuner()
{
}

string ConfRuner::getKeyValue(const string& sectionName, const string& key)
{
	auto pr = m_multimap_contents.equal_range(sectionName);
	if (pr.first != m_multimap_contents.end())
	{
		for (auto kv = pr.first; kv != pr.second; ++kv)
		{
			if (kv->second.key == key)
				return kv->second.value;
		}
	}
	return "";
}

bool ConfRuner::confLoad(std::string path)
{
	m_path = path;
	CSimpleIniA ini;
	ini.SetUnicode(true);
	if (0 > ini.LoadFile(m_path.c_str()))
		return false;
	list <CSimpleIniA::Entry> sections;
	ini.GetAllSections(sections);
	for (auto infoSection : sections)
	{
		list <CSimpleIniA::Entry> keys;
		keyVal temp;
		if (!ini.GetAllKeys(infoSection.pItem, keys))
		{
			temp.key = temp.value = "";
			m_multimap_contents.insert(std::make_pair(infoSection.pItem, temp));
			continue;
		}
		for (auto infoKey : keys)
		{
			temp.key = infoKey.pItem;
			temp.value = ini.GetValue(infoSection.pItem, infoKey.pItem);
			m_multimap_contents.insert(std::make_pair(infoSection.pItem, temp));
		}
	}
	return true;
}

bool ConfRuner::confReload()
{
	confLoad(m_path);
	return 0;
}