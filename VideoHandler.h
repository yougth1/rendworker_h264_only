#pragma once
#include <string>
#include "common.h"
#include <vector>
#include <thread>
#include <map>
using namespace std;
struct scenicSpotsParams
{
	bool skipTheFirstSecond;
	bool beautifyFlag;
	string eye;
	string chin;
	string red_face;
	string white_skin;
	string skin_smooth;
};

class VideoHandler final
{
public:
	enum toolType
	{
		FFMPEG
	};
public:
	bool init(int movieToolType);
	bool cutVid(const string& srcVidPath, string& outPath, const string& scenicSpotsName);
	bool beautify(const string& srcVidPath, string& outPath, const string& scenicSpotsName);
	bool overlay(const string& srcVidPath, const string& subVidPath, const string& outPath);
	bool combineVids(const string& outFolder, const vector<string>& sorted_filePaths, const string& outFilePath);
	string makeCmdPath(string strPath);
	bool addSubtitle(const string& vidPath, const string& subFilePath, const string& subContent, const string& outFolder, const string& outFilePath);
	string replaceAssFile(const string& subFilePath, const string& outFolder, const string& subContent);
	bool addAudio(const string& vidPath, const string& audioPath, const string& outFilePath);
	bool waterMark(const string& vidPath, const string& code, const string& outFolder);
	bool isFileExists(string& name);
	bool mp4Tots(const string& srcPath, const string& outPath);
	bool cubeFilter(const string& vidPath, const string& cubePath, const string& outFilePath);
	bool changeSpeed(const string& srcVidPath, const string& outVidPath, const int& currentVidFps, const float& speedRate);
	void close();

	bool makeVidListFile(const string& vidsFolder, const vector<string>& sorted_filePaths);
	static VideoHandler& instanse();

private:
	bool exeCmd(const string& cmd);
private:
	VideoHandler() {};
	VideoHandler(VideoHandler&);
	VideoHandler& operator=(VideoHandler&) {};
	string getMovieTool(int toolType);
	long hex2int(const string& hexStr);
	wstring stringTowstring(const string& strSrc);
	wstring replaceVal(const wstring& src, const wstring& rps, const wstring& tempString);
	wstring transeWContents(const string& subTileSrc, const string fileContents);
	string m_currentDir;
	string m_currentMovieTool;
	string m_subTitleTempWords;
	map<string, scenicSpotsParams> m_map_bParam;
};

