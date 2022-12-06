#include "VideoHandler.h"
#include <fstream>
#include <io.h>    
#include <sstream>
#include "LogManager.h"
#include "confmaner.h"
bool VideoHandler::init(int movieToolType)
{
	m_currentDir = GetCurrentPath();
	m_currentMovieTool = getMovieTool(movieToolType);

	std::string scenicSpots = ConfRuner::instance().getKeyValue("sub_params", "scenic_spots");
	std::stringstream ss(scenicSpots);
	std::string item;
	while (std::getline(ss, item, ','))
	{
		scenicSpotsParams param;
		string section = item + "_param";
		param.skipTheFirstSecond = ("0" == ConfRuner::instance().getKeyValue(section, "skip_the_first_second")) ? false : true;
		param.beautifyFlag = ("0" == ConfRuner::instance().getKeyValue(section, "beautify")) ? false : true;
		param.eye = ConfRuner::instance().getKeyValue(section, "eye");
		param.chin = ConfRuner::instance().getKeyValue(section, "chin");
		param.red_face = ConfRuner::instance().getKeyValue(section, "red_face");
		param.white_skin = ConfRuner::instance().getKeyValue(section, "white_skin");
		param.skin_smooth = ConfRuner::instance().getKeyValue(section, "skin_smooth");
		m_map_bParam.insert(make_pair(item, param));
	}
	return true;
}
bool VideoHandler::cutVid(const string& srcVidPath, string& outPath, const string& scenicSpotsName)
{
	auto it = m_map_bParam.find(scenicSpotsName);
	if (m_map_bParam.end() == it)
	{
		LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]VHError : invalid scenic spots name \"%s\"", scenicSpotsName.c_str());
		return false;
	}
	cout << "SPFlag =========== " << it->second.skipTheFirstSecond << endl;
	if (!(it->second.skipTheFirstSecond))
	{
		cout << "[rendworker] : Current scenic spots will skip cutVid!" << endl;
		outPath = srcVidPath;
		return true;
	}
	string cmd = "-ss 00:00:01 -i " + srcVidPath + " -c copy -y " + outPath;
	return exeCmd(cmd);
	
}

bool VideoHandler::beautify(const string& srcVidPath, string& outPath, const string& scenicSpotsName)
{
	auto it = m_map_bParam.find(scenicSpotsName);
	if (m_map_bParam.end() == it)
	{
		LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]VHError : invalid scenic spots name \"%s\"", scenicSpotsName.c_str());
		return false;
	}
	if (!(it->second.beautifyFlag))
	{
		cout << "[rendworker] : Current scenic spots will skip beautify!" << endl;
		outPath = srcVidPath;
		return true;
	}
	scenicSpotsParams sp = it->second;
	cout << "[rendworker] : Beautify video \'" << srcVidPath << "\'...[timestamp : " << time(NULL) << "]" << endl;
	string cmd = m_currentDir + "\\xjg.exe -i " + srcVidPath + " -e " + sp.eye + " -c " + sp.chin + " -r " + sp.red_face + " -w "
		+ sp.white_skin + " -s " + sp.skin_smooth + " -o " + outPath;
	bool res = (system(cmd.c_str()) == 0 ? true : false);
	cout << "[rendworker] :  Beautify video \'" << outPath << "\' sucessed! [timestamp : " << time(NULL) << "]" << endl;
	return res;
}

bool VideoHandler::overlay(const string& srcVidPath, const string& subVidPath, const string& outPath)
{
	cout << "[rendworker] : Overlaying video \'" << srcVidPath << "\'...[timestamp : " << time(NULL) << "]" << endl;
	string cmd = "-i " + srcVidPath + " -i " + subVidPath + " -filter_complex \"overlay=1:eof_action=endall\" -r 25 -c:v h264_qsv -an -b:v 6M -video_track_timescale 25k -y " + outPath;
	bool res = exeCmd(cmd);
	cout << "[rendworker] : Overlay video \'" << outPath << "\' sucessed! [timestamp : " << time(NULL) << "]" << endl;
	return res;
}

bool VideoHandler::combineVids(const string& outFolder, const vector<string>& sorted_filePaths, const string& outFilePath)
{
		if (!makeVidListFile(outFolder, sorted_filePaths))
			return false;

		string cmd = "-f concat  -safe 0 -i " + outFolder + "vidlist -b:v 6M -vcodec copy -an -y " + outFilePath;
		//string cmd = "-f concat  -safe 0 -i " + outFolder + "vidlist -vcodec copy -an -y " + outFilePath;
		return exeCmd(cmd);

}

bool VideoHandler::cubeFilter(const string& vidPath, const string& cubePath, const string& outFilePath)
{
	string cube = makeCmdPath(cubePath);
	string cmdString = "-i " + vidPath + " -filter_complex lut3d=file=" + cube + " -y " + outFilePath;
	return exeCmd(cmdString);
}

bool VideoHandler::changeSpeed(const string& srcVidPath, const string& outVidPath, const int& currentVidFps, const float& speedRate)
{
	int fps = currentVidFps * speedRate;
	string cmdString = "-i " + srcVidPath + " -r " + to_string(fps) + R"( -filter:v "setpts=)" + to_string(speedRate) + R"(*PTS" -y )" + outVidPath;
	return exeCmd(cmdString);
}

string VideoHandler::makeCmdPath(string strPath)
{
	//string newPath = "E:\\codes\\rendworker\\x64\\Release\\templates\\ggh/2.0-30s-3g\\assets\\logo.png";

	size_t pos = 0;
	while ((pos = strPath.find("\\", pos)) != string::npos)
	{
		strPath.replace(pos, 1, "/");
		++pos;
	}
	if ((pos = strPath.find(":", 0)) != string::npos)
	{
		strPath.replace(pos, 1, "\\\\:\\");
		//strPath.erase(pos + 2, 1);
	}
	return strPath;
}

bool VideoHandler::addSubtitle(const string& vidPath, const string& subFilePath, const string& subContent, const string& outFolder, const string& outFilePath)
{
	string newAss = replaceAssFile(subFilePath, outFolder, subContent);
	string newAssPath = "";
	for (auto s : newAss) 
	{
		if (s == '\\')
		{
 			newAssPath.push_back(s);
			newAssPath.push_back(s);
		}
		else if (s == ':') 
		{
			newAssPath.push_back('\\');
			newAssPath.push_back(s);
		}
		else
		{
			newAssPath.push_back(s);
		}
	}
	//newAss = makeCmdPath(newAss);
	//string cmd = "-i "+ vidPath + " -vf subtitles=\'" + newAssPath + "\' -r 25 -c:v h264_qsv -an -video_track_timescale 25k -y " + outFolder + outFileName;
	string cmd = "-i " + vidPath + " -vf subtitles=\'" + newAssPath + "\' -c:v h264_qsv -an -b:v 6M -video_track_timescale 25k -y " + outFilePath;
	return exeCmd(cmd);
}

string VideoHandler::replaceAssFile (const string& subFilePath, const string& outFolder, const string& subContent)
{
	string newAss = outFolder + "replacedAss.ass";
	ifstream inFile(subFilePath);
	if(!inFile.is_open())
	{
		LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error][Rendworker Error]Can not open file :\"%s\"", subFilePath.c_str());
		return "";
	}

	istreambuf_iterator<char> beg(inFile), end;
	string strSubContents(beg, end);
	inFile.close();
	strSubContents = UTF8ToGBK(strSubContents);
	strSubContents = GBKToUTF8(strSubContents);
	if (0 == strSubContents.find("?"))
	{
		strSubContents.erase(0, 1);
	}
	FILE* fp = _wfsopen(stringTowstring(newAss).c_str(), L"w+,ccs=UTF-8", _SH_DENYNO);
	if (fp == NULL)
	{
		printf("err\n");
	}
	fwprintf_s(fp, L"%s", transeWContents(subContent, strSubContents).c_str());
	fclose(fp);
	return newAss;
}

bool VideoHandler::addAudio(const string& vidPath, const string& audioPath, const string& outFilePath)
{
	string cmd = "-i " + vidPath + " -i " + audioPath + " -c copy -shortest -y " + outFilePath;
	return exeCmd(cmd);
}

bool VideoHandler::waterMark(const string& vidPath, const string& watermarkPath, const string& outFolder)
{
	string newWaterMarkPath = watermarkPath;
	newWaterMarkPath = makeCmdPath(newWaterMarkPath);
 	string cmd = "-i " + vidPath + " -vf \"movie=" + newWaterMarkPath + "[watermark];[in][watermark]overlay=main_w-overlay_w-10:main_h-overlay_h-10 [out]\" -y " + outFolder + "_marked.mp4";
	return exeCmd(cmd);
}

bool VideoHandler::isFileExists(string& name)
{
	ifstream f(name.c_str());
	return f.good();
}

bool VideoHandler::mp4Tots(const string& srcPath, const string& outPath)
{
	string cmd = "-i " + srcPath + " -vcodec copy -acodec copy -vbsf h264_mp4toannexb -y " + outPath;
	//string cmd = "-i " + srcPath + " -vcodec copy -acodec copy -y " + outPath;
	return exeCmd(cmd);
}

void VideoHandler::close()
{
	m_currentDir = "";
	m_currentMovieTool = "";
}

string VideoHandler::getMovieTool(int toolType)
{
	switch (toolType)
	{
	case FFMPEG:
		return (m_currentDir + "\\ffmpeg.exe  -loglevel quiet -hwaccel qsv ");
		//return (m_currentDir + "\\ffmpeg-b4.2.2.exe -loglevel quiet -hwaccel qsv ");
		break;
	default:
		return "";
	}
}
long VideoHandler::hex2int(const string& hexStr)
{
	char* offset;
	if (hexStr.length() > 2)
	{
		if (hexStr[0] == '0' && hexStr[1] == 'x')
		{
			return strtol(hexStr.c_str(), &offset, 0);
		}
	}
	return strtol(hexStr.c_str(), &offset, 16);
}
wstring VideoHandler::stringTowstring(const string& strSrc)
{
	wchar_t wide[100000] = { L"" };
	MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, wide, strSrc.length());
	return wstring(wide);
}

wstring VideoHandler::replaceVal(const wstring& src, const wstring& rps, const wstring& tempString)
{
	wstring str = src;
	size_t pos = 0;
	while (wstring::npos != (pos = str.find(tempString, pos)))
	{
		str = str.replace(pos, tempString.length(), rps);
	}
	return str;
}

wstring VideoHandler::transeWContents(const string& subTileSrc, const string fileContents)
{
	string strTemp;
	string emojiString = "";

	size_t startPos = 0;
	size_t subPos = 0;
	size_t emojiPos = 0;
	wstring w_str = stringTowstring(fileContents);
	wstring usr_name = L"";
	wstring 😈 = L"";
	while (true)
	{
		if (string::npos == (subPos = subTileSrc.find(R"(\u)", startPos)))
		{
			if (startPos == 0)
			{
				usr_name = stringTowstring(subTileSrc);
				break;
			}
			strTemp = subTileSrc.substr(startPos, subTileSrc.length() - startPos);
			usr_name = usr_name + stringTowstring(strTemp);
			break;
		}
		else if (subPos != startPos)
		{
			strTemp = subTileSrc.substr(startPos, subPos - startPos);
			usr_name = usr_name + stringTowstring(strTemp);
		}
		startPos = subPos + 2;
		subPos += 6;
		emojiString = subTileSrc.substr(startPos, subPos - startPos);
		😈 = hex2int(emojiString);
		usr_name = usr_name + 😈;
		startPos = subPos;
	}
	return replaceVal(w_str, usr_name, L"ZCYY_SWAP_TEMP_ASS");
}

bool VideoHandler::makeVidListFile(const string& outFolder, const vector<string>& sorted_filePaths)
{
	string strVidlistPath = outFolder + "\\vidlist";
	ofstream outFile(strVidlistPath);
	if (!outFile.is_open())
	{
		LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]", "Can not open vidlist");
		return false;
	}
	string filter = "file \'";
	for(auto strFilePath : sorted_filePaths)
	{
		outFile << filter << strFilePath << "\'" << endl;
	}
	outFile.close();
	return true;
}

VideoHandler& VideoHandler::instanse()
{
	static VideoHandler vh;
	return vh;
}

bool VideoHandler::exeCmd(const string& cmd)
{
	string finalCmd = m_currentMovieTool + cmd;
	int res = system(finalCmd.c_str());
	if (res != 0)
	{
		LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Error command : [%s]", finalCmd.c_str());
		return false;
	}
	return true;
}
