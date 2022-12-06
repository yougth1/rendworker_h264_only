#pragma once
#include <string>
#include <vector>
using namespace std;

struct audioInfos
{
	string vidPath;
	string audioPath;
	string outFolder;
};

struct postrender
{
	string output;
	string input;
	string bucket;
};

struct finalTaskParams
{
	audioInfos audioParams;
	string watermark_location;
	postrender postParams;
    string sessionid;
};



struct subInfo
{
	string property;
	string layerName;
	string type;
	string value;
};

struct vedioInfo
{
	string src;
	string layerName;
	string bucket;
	string type;
};

struct contentsAssets
{
	vector<vedioInfo> vec_vidInfos;
	subInfo assContent;
};

struct ossInfo
{
	string access_key_id;
	string access_key_secret;
	string region;
};

struct dataContents
{
	string templateSrc;
	ossInfo ossinfos;
	contentsAssets assets;
	string sessionid;
	vector<postrender> vec_postrenders;
};

struct missionData
{
	string  uid;
	dataContents contents;
};