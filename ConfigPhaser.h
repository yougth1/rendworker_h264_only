#ifndef _CONFIG_PHASER_H_
#define _CONFIG_PHASER_H_

#include "common.h"
#include <vector>
#include <cJSON.h>

#if 0
typedef struct _LogoInfo {
	string location;
}LogoInfo;
#endif
typedef struct _OverlayVideo {
	string location;
	string afterOverlayLocation;
}OverlayVideo;

typedef struct _Video {
	int pos;
	string type;
	string location;
	int overlay;
	int filter;
	int beautify;
	string overlayFile_location;
	string fliterFile_location;
}Video;

typedef struct _VideoInfo {
	int videoNum;
	vector<Video> videos;
}VideoInfo;


typedef struct _AudioInfo {
	string location;
}AudioInfo;

typedef struct _AssInfo {
	int pos;
	string location;
}AssInfo;

typedef struct _ConfigInfo {
	VideoInfo videoInfo;
	string audioLocation;
	//AudioInfo audioInfo;
	AssInfo assInfo;
	//LogoInfo logoInfo;
	string waterMarkLocation;
}ConfigInfo;


class ConfigPhaser
{
private:
	static ConfigPhaser *local_instance;
	ConfigPhaser() {};

	static ConfigInfo *configInfo;

public:
	static ConfigPhaser *getInstance()
	{
		if (local_instance == nullptr)
		{
			local_instance = new ConfigPhaser();
		}
		return local_instance;
	}


	int getIntValue(cJSON* obj, const string& key);

	string getStringValue(cJSON* obj, const string& key);

	bool phase(const string &templateFolderPath, const string & scenicSpotsFolderPath);
	ConfigInfo * getConfig();
	
};

#endif



