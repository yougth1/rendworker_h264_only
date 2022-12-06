#include "ConfigPhaser.h"
#include <fstream>
#include "LogManager.h"


ConfigPhaser* ConfigPhaser::local_instance = nullptr;
ConfigInfo* ConfigPhaser::configInfo = nullptr;

int ConfigPhaser::getIntValue(cJSON* obj, const string& key)
{
	cJSON* item = cJSON_GetObjectItem(obj, key.c_str());
	if (item == nullptr)
		return 0;
	return item->valueint;
}

string ConfigPhaser::getStringValue(cJSON* obj, const string& key)
{
	cJSON* item = cJSON_GetObjectItem(obj, key.c_str());
	if (item == nullptr)
		return "";
	return item->valuestring;
}

bool ConfigPhaser::phase(const string& templateFolderPath, const string& scenicSpotsFolderPath)
{
	if (configInfo != nullptr)
	{
		delete configInfo;
	}
	string strConfigFile = templateFolderPath + scenicSpotsFolderPath + "\\config.json";
	string strLocalPath = templateFolderPath + scenicSpotsFolderPath + + "\\";

	ifstream ifs(strConfigFile);
	if (!ifs.is_open())
	{
		LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Can not open template file :\"%s\"", strConfigFile.c_str());
		return false;
	}

	istreambuf_iterator<char> beg(ifs), end;
	string strConfig(beg, end);
	//cout << "ÅäÖÃÎÄ¼þÄÚÈÝ:" << endl;
	//cout << strConfig << endl;
	ifs.close();
	cJSON* jsonConfig = cJSON_Parse(strConfig.c_str());
	if (jsonConfig == nullptr)
	{
		LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Invalid tmeplate file : can not get root object from file :\"%s\"", strConfigFile.c_str());
		return false;
	}
	configInfo = new ConfigInfo();
	{
		/////////////////////parse video infos//////////////////////////
		cJSON* itemVideoInfo = cJSON_GetObjectItem(jsonConfig, "videoInfo");
		if (itemVideoInfo == nullptr)
		{
			LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Invalid tmeplate file : no \'videoinfo\' in file :\"%s\"", strConfigFile.c_str());
			goto ERRORPOINT;
		}
	
		configInfo->videoInfo.videoNum = getIntValue(itemVideoInfo, "videoNum");
		if (configInfo->videoInfo.videoNum == 0)
		{
			LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Invalid tmeplate file : 0 videos in template file :\"%s\"", strConfigFile.c_str());
			goto ERRORPOINT;
		}

		vector<string> filterLocations;
		int arraySize = 0;
#ifdef OVERLAY_CHOOSE
		vector<string> overlayLocations;
		cJSON* itemOverlayInfos = cJSON_GetObjectItem(jsonConfig, "overlayInfo");
		if (itemOverlayInfos == nullptr)
		{
			LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]Invalid tmeplate file : no overlay contents in file :\"%s\"", strConfigFile.c_str());
			goto ERRORPOINT;
		}
		arraySize = cJSON_GetArraySize(itemOverlayInfos);
		for (int i = 0; i < arraySize; ++i)
		{
			cJSON* itemOverlay = cJSON_GetArrayItem(itemOverlayInfos, i);
			if (itemOverlayInfos == nullptr)
			{
				LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]Invalid tmeplate file : invalid overlay infos in file :\"%s\"", strConfigFile.c_str());
				goto ERRORPOINT;
			}
			overlayLocations.push_back(itemOverlay->valuestring);
		}
#endif
		do {
			cJSON* itemFilterInfos = cJSON_GetObjectItem(jsonConfig, "filterInfo");
			if (itemFilterInfos == nullptr)
			{
				//LogManager::instance().log(LogManager::LV_WARNING, "", "Tmeplate warning : no filter info in file :\"%s\"", strConfigFile.c_str());
				break;
			}
			arraySize = cJSON_GetArraySize(itemFilterInfos);
			for (int i = 0; i < arraySize; ++i)
			{
				cJSON* itemfFilter = cJSON_GetArrayItem(itemFilterInfos, i);
				if (itemfFilter == nullptr)
				{
					LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Invalid tmeplate file : invalid filter infos in file :\"%s\"", strConfigFile.c_str());
					goto ERRORPOINT;
				}
				filterLocations.push_back(itemfFilter->valuestring);
			}
		} while (false);

		cJSON* itemVideos = cJSON_GetObjectItem(itemVideoInfo, "videos");
		if (itemVideos == nullptr)
		{
			LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Invalid tmeplate file : no video contents in file :\"%s\"", strConfigFile.c_str());
			goto ERRORPOINT;
		}
		arraySize = cJSON_GetArraySize(itemVideos);
		for (int i = 0; i < arraySize; ++i)
		{
			cJSON* itemVideo = cJSON_GetArrayItem(itemVideos, i);
			if (itemVideo == nullptr)
			{
				LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Invalid tmeplate file : invalid video infos in file :\"%s\"", strConfigFile.c_str());
				goto ERRORPOINT;
			}
			Video video;
			video.pos = getIntValue(itemVideo, "pos");
			video.type = getStringValue(itemVideo, "type");
			if (video.type == "scene")
			{
				video.location = strLocalPath + cJSON_GetObjectItem(itemVideo, "location")->valuestring;
			}
			else if (video.type == "user")
			{
#ifdef OVERLAY_CHOOSE
				if (0 < (video.overlay = getIntValue(itemVideo, "overlay")))
				{
					string location = overlayLocations.at(video.overlay - 1);
					video.overlayFile_location = (location != "" ? strLocalPath + location : "");
				}
				if (0 < (video.filter = getIntValue(itemVideo, "filter")))
				{
					string location = filterLocations.at(video.filter - 1);
					video.fliterFile_location = (location != "" ? strLocalPath + location : "");
				}
#else
				string location = getStringValue(itemVideo, "overlay");
				video.overlayFile_location = (location != "" ? strLocalPath + location : "");
				if (0 < (video.filter = getIntValue(itemVideo, "filter")))
				{
					string location = filterLocations.at(video.filter - 1);
					video.fliterFile_location = (location != "" ? strLocalPath + location : "");
				}
				video.beautify = getIntValue(itemVideo, "beautify");
#endif
			}
			else
			{
				LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Invalid tmeplate file : invalid video type in file :\"%s\"", strConfigFile.c_str());
				goto ERRORPOINT;
			}
			configInfo->videoInfo.videos.push_back(video);
		}
	}
	{
		/////////////////////parse audio infos//////////////////////////
		cJSON* itemAudioLocation = cJSON_GetObjectItem(jsonConfig, "audio_location");
		if (itemAudioLocation == nullptr)
			configInfo->audioLocation = "";
		else
			configInfo->audioLocation = strLocalPath + itemAudioLocation->valuestring;
	}
	{
		/////////////////////parse ass infos//////////////////////////
		cJSON* itemAssInfo = cJSON_GetObjectItem(jsonConfig, "assInfo");
		if (itemAssInfo == nullptr)
		{
			//LogManager::instance().log(LogManager::LV_WARNING, "", "Tmeplate warning : no subtitle info in file : \"%s\"", strConfigFile.c_str());
			cout << "templates warning : !!!" << endl;
			configInfo->assInfo.location = "";
		}
		else
		{
			cJSON* itemPos = cJSON_GetObjectItem(itemAssInfo, "pos");
			cJSON* itemLocation = cJSON_GetObjectItem(itemAssInfo, "location");
			if (itemPos == nullptr || itemLocation == nullptr)
			{
				LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Invalid subtitle info in file : \"%s\"", strConfigFile.c_str());
				goto ERRORPOINT;
			}
			else if (itemPos->valueint > configInfo->videoInfo.videoNum)
			{
				LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]Invalid subtitle file pos in file : \"%s\"", strConfigFile.c_str());
				cout << "!" << endl;
				goto ERRORPOINT;
			}
			configInfo->assInfo.pos = itemPos->valueint;
			configInfo->assInfo.location = strLocalPath + itemLocation->valuestring;
		}
	}
	{
		/////////////////////parse waterMark infos//////////////////////////
		cJSON* itemWatermarkLocation = cJSON_GetObjectItem(jsonConfig, "watermark_name");
		if (itemWatermarkLocation == nullptr)
			configInfo->waterMarkLocation = "";
		else
			configInfo->waterMarkLocation = templateFolderPath + itemWatermarkLocation->valuestring;
	}
#if 0
	{
		/////////////////////parse logo infos//////////////////////////
		cJSON* itemLogoInfo = cJSON_GetObjectItem(jsonConfig, "logoInfo");
		if (itemLogoInfo == nullptr)
		{
			LogManager::instance().log(LogManager::LV_WARNING, "No logo info in file : \"%s\"", strConfigFile.c_str());
			configInfo->logoInfo.location = "";
		}
		else
		{
			cJSON* itemLocation = cJSON_GetObjectItem(itemLogoInfo, "location");
			if (itemLocation == nullptr)
			{
				LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]Invalid logo info in file : \"%s\"", strConfigFile.c_str());
				goto ERRORPOINT;
			}
			configInfo->logoInfo.location = strLocalPath + itemLocation->valuestring;
		}
	}
#endif
	return true;
	{
	ERRORPOINT:
		if (jsonConfig != nullptr)
			cJSON_Delete(jsonConfig);
		if (configInfo != nullptr)
		{
			delete configInfo;
			configInfo = nullptr;
		}
		LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Task error end!");
		return false;
	}
}

ConfigInfo* ConfigPhaser::getConfig()
{
	return configInfo;
}
