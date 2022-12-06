#include <map>
#include <time.h>
#include <iostream>
#include <queue>
#include "ThreadManager.h"
#include "LogManager.h"
#include "confmaner.h"
#include "NetCom.h"

#include "alisdk.h"
#include "VideoHandler.h"
//#include <python.h>
#define BEAUTIFY_INI_SET
using namespace std;
bool ThreadManager::_runFlag = false;
volatile bool taskFlag = true;

std::mutex ex;

void ThreadManager::startWork()
{
	_runFlag = true;
	if (_thread != nullptr)
	{
		delete _thread;
	}
	_thread = new std::thread(handleEvent);
}

bool ThreadManager::init()
{
	do {
		if (!ConfRuner::instance().confLoad(GetCurrentPath() + "\\..\\config.ini"))
		{
			// LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]Config file :\"%s\" load failed!", (GetCurrentPath() + "\\config.ini").c_str());
			cout << "Config file load failed!!!!!!!!! " << endl;
			break;
		}
		if (!LogManager::instance().init((GetCurrentPath() + "\\..\\log\\rendworker").c_str()))
		{
			// LogManager::instance().log(LogManager::LV_ERROR, "[Rendworker Error]Config file :\"%s\" load failed!", (GetCurrentPath() + "\\config.ini").c_str());
			cout << "Log init failed!!!!!!!!! " << endl;
			break;
		}
		//Py_Initialize();
		if (NetCom::instanse().init() && VideoHandler::instanse().init(VideoHandler::toolType::FFMPEG))
		{
			LogManager::instance().log(LogManager::LV_INFO, "", "Init complete!");
			return true;
		}
	} while (false);
	LogManager::instance().log(LogManager::LV_ERROR, "", "[Rendworker Error]Net init failed!");
	close();
	return false;
}

void ThreadManager::start()
{
	if (_runFlag)
		stop();
	startWork();
}

void ThreadManager::waitForTasksEnd()
{
	cout << "[rendworker] : Waiting for tasks end..." << endl;
	for (auto th : m_vec_tasks)
	{
		th->join();
		delete th;
	}
	m_vec_tasks.clear();
	cout << "[rendworker] : Tasks ended! [timestamp : " << time(NULL) << "]" << endl;
}

void ThreadManager::addUsrVidTask(userTaskParam param)
{
	m_vec_tasks.push_back(new thread(usrVidTask, param));
}

void ThreadManager::usrVidTask(userTaskParam param)
{
	//string localSavePath = localDownloadDirPath + data.contents.templateSrc + "\\assets\\" ;
	string tmpPath = param.localUpdateDirPath + param.vidInfo.layerName + ".mp4";
	string scenicSpotsName = param.data.contents.templateSrc.substr(0, param.data.contents.templateSrc.find("/"));
	string outPath = param.localUpdateDirPath + "cuted_" + param.vidInfo.layerName + ".mp4";
	cout << "[rendworker] : Downlaoding resource \'" << param.vidInfo.src << "\' [timestamp : " << time(NULL) << "]" << endl;
	string errStr = "";
	if ("" != (errStr = AliSDKHandle::GetInstance()->AliSDKDownloadHandle(GBKToUTF8(param.vidInfo.bucket), GBKToUTF8(param.vidInfo.src), tmpPath)))
		// if ("" != (errStr = AliSDKHandle::GetInstance()->AliSDKDownloadHandle(GBKToUTF8(param.vidInfo.bucket), GBKToUTF8(param.vidInfo.src), tmpPath)))
	{
		std::lock_guard<std::mutex> lockg(mutex);
		LogManager::instance().log(LogManager::LV_ERROR, param.data.contents.sessionid, "[Rendworker Error]Failed to download file :\"%s\", key_id :\"%s\", bucket :\"%s\", key_secret :\"%s\", region : \"%s\", secreaterrorInfos:\"%s\"", param.vidInfo.src.c_str(), param.data.contents.ossinfos.access_key_id.c_str(),
			param.vidInfo.bucket.c_str(), param.data.contents.ossinfos.access_key_secret.c_str(), param.data.contents.ossinfos.region.c_str(), errStr.c_str());
		//NetCom::instanse().sendState(param.data.uid, NetCom::stateType::error, "Failed to download file \"" + param.vidInfo.src + "\"");
		taskFlag = false;
		return;
	}
	param.configInfo->videoInfo.videos[param.vid.pos - 1].location = tmpPath;

	{
		if (!VideoHandler::instanse().cutVid(tmpPath, outPath, scenicSpotsName))
		{
			std::lock_guard<std::mutex> lockg(mutex);
			LogManager::instance().log(LogManager::LV_ERROR, param.data.contents.sessionid, "[Rendworker Error]Failed to cut file :\"%s\"", param.vidInfo.src.c_str());
			//NetCom::instanse().sendState(param.data.uid, NetCom::stateType::error, "Failed to cut file \"" + param.vidInfo.src + "\"");
			LogManager::instance().copyErrorFile(tmpPath, "_cutError.mp4");
			taskFlag = false;
			return;
		}
		tmpPath = outPath;
		param.configInfo->videoInfo.videos[param.vid.pos - 1].location = tmpPath;
	}

#ifdef BEAUTIFY_JSON_SET
	if (vec_userVids.at(index).beautify != 0)
#endif // def BEAUTIFY_JSON_SET
	{
		outPath = param.localUpdateDirPath + "beautied" + param.vidInfo.layerName + ".mp4";
		if (!VideoHandler::instanse().beautify(tmpPath, outPath, scenicSpotsName))
		{
			std::lock_guard<std::mutex> lockg(mutex);
			LogManager::instance().log(LogManager::LV_ERROR, param.data.contents.sessionid, "[Rendworker Error]Failed to beauty file :\"%s\"", param.vidInfo.src.c_str());
			// NetCom::instanse().sendState(param.data.uid, NetCom::stateType::error, "Failed to beauty file \"" + param.vidInfo.src + "\"");
			taskFlag = false;
			return;
		}
		tmpPath = outPath;
		param.configInfo->videoInfo.videos[param.vid.pos - 1].location = tmpPath;
	}
	if (param.vid.filter > 0)
	{
		cout << "[rendworker] : Filter video \'" << tmpPath << "\'...[timestamp : " << time(NULL) << "]" << endl;
		string filterPath = param.vid.fliterFile_location;
		string outPath = param.localUpdateDirPath + "filtered_" + param.vidInfo.layerName + ".mp4";
		if (!VideoHandler::instanse().cubeFilter(tmpPath, filterPath, outPath))
		{
			std::lock_guard<std::mutex> lockg(mutex);
			LogManager::instance().log(LogManager::LV_ERROR, param.data.contents.sessionid, "[Rendworker Error]Failed to filter file :\"%s\"", tmpPath.c_str());
			//NetCom::instanse().sendState(param.data.uid, NetCom::stateType::error, "Failed to filter file \"" + tmpPath + "\"");
			taskFlag = false;
			return;
		}
		tmpPath = outPath;
		cout << "[rendworker] : Filter video \'" << tmpPath << "\' sucessed! [timestamp : " << time(NULL) << "]" << endl;
		param.configInfo->videoInfo.videos[param.vid.pos - 1].location = tmpPath;
	}

	/////背景虚化
	{
#if 0
		PyObject* pModule = NULL;//声明变量
		PyObject* pFunc = NULL;// 声明变量
		pModule = PyImport_ImportModule("bokehProc");//这里是要调用的文件名
		pFunc = PyObject_GetAttrString(pModule, "bokeh_proc");//这里是要调用的函数名
		PyEval_CallObject(pFunc, );//调用函数
		tmpPath = outPath;
#endif
	}
#ifdef OVERLAY_CHOOSE
	if (info.overlay > 0)
#else
	if (param.vid.overlayFile_location != "")
#endif
	{
		string subPath = param.vid.overlayFile_location;
		string outPath = param.localUpdateDirPath + "overlay_" + param.vidInfo.layerName + ".mp4";
		if (!VideoHandler::instanse().overlay(tmpPath, subPath, outPath))
		{
			LogManager::instance().log(LogManager::LV_ERROR, param.data.contents.sessionid, "[Rendworker Error]Failed to overlay file :\"%s\"", tmpPath.c_str());
			NetCom::instanse().sendState(param.data.uid, NetCom::stateType::error, "Failed to overlay file \"" + tmpPath + "\"");
			taskFlag = false;
			return;
		}
		param.configInfo->videoInfo.videos[param.vid.pos - 1].location = outPath;
	}
}
void ThreadManager::finalTask(finalTaskParams ftps)
{
	string finalVidPath = ftps.audioParams.outFolder + "FinalVideo_noMark.mp4";
	if (ftps.watermark_location != "")
	{
		cout << "[rendworker] : Overlaying watermark to file \"" << ftps.audioParams.vidPath << "\"! [timestamp : " << time(NULL) << "]" << endl;
		if (!VideoHandler::instanse().overlay(ftps.audioParams.vidPath, ftps.watermark_location, ftps.audioParams.outFolder + "waterMarked.mp4"))
		{
			LogManager::instance().log(LogManager::LV_ERROR, ftps.sessionid, "[Rendworker Error]Failed to overlay watermark to file :\"%s\"", ftps.audioParams.vidPath.c_str());
			taskFlag = false;
			return;
		}
		ftps.audioParams.vidPath = ftps.audioParams.outFolder + "waterMarked.mp4";
		finalVidPath = ftps.audioParams.outFolder + "FinalVideo_marked.mp4";
		cout << "[rendworker] : Watermark added![timestamp:" << time(NULL) << "]" << endl;
	}
	if (ftps.audioParams.audioPath != "")
	{
		cout << "[rendworker] : Adding audio to file \"" << ftps.audioParams.vidPath << "\"! [timestamp : " << time(NULL) << "]" << endl;
		if (!VideoHandler::instanse().addAudio(ftps.audioParams.vidPath, ftps.audioParams.audioPath, finalVidPath))
		{
			LogManager::instance().log(LogManager::LV_ERROR, ftps.sessionid, "[Rendworker Error]Failed to add audio to file \"%s\"!", ftps.audioParams.vidPath.c_str());
			//  NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Failed to add audio[mark]");
			taskFlag = false;
			return;
		}
		cout << "[rendworker] : Audio added to file \"" << ftps.audioParams.vidPath << "\"![timestamp:" << time(NULL) <<  "]" << endl;
	}
	cout << "[rendworker] : Uploading file \'" << finalVidPath << "\' [timestamp : " << time(NULL) << "]" << endl;
	string errStr = "";
	if ("" != (errStr = AliSDKHandle::GetInstance()->AliSDKUploadHandle(GBKToUTF8(ftps.postParams.bucket), finalVidPath, GBKToUTF8(ftps.postParams.output))))
	{
		std::lock_guard<std::mutex> lockg(mutex);
		LogManager::instance().log(LogManager::LV_ERROR, ftps.sessionid,
			"[Rendworker Error]Failed to update file :\"%s\", errorInfos:\"%s\"", finalVidPath.c_str(), errStr.c_str());
		taskFlag = false;
		return;
	}
	cout << "[rendworker] : Upload file complete! [timestamp : " << time(NULL) << "]" << endl;
}

void ThreadManager::addFinalTask(finalTaskParams ftps)
{
	m_vec_tasks.push_back(new thread(finalTask, ftps));
}

void ThreadManager::handleEvent()
{
	missionData data;
	string currentPath;
	//string localUpdateDirPath = GetCurrentPath() + "\\tmp\\";
	//string localTemplatesDirPath = GetCurrentPath() + "\\templates\\";
	string localUpdateDirPath = GetCurrentPath() + "\\" + ConfRuner::instance().getKeyValue("fileOption", "upload_folder") + "\\";
	string localTemplatesDirPath = GetCurrentPath() + "\\" + ConfRuner::instance().getKeyValue("fileOption", "templates_folder") + "\\";
	ConfigInfo* configInfo = nullptr;
	string ossAddr = ConfRuner::instance().getKeyValue("rendworker", "ossAddr");
	string subtitle_temp = ConfRuner::instance().getKeyValue("fileOption", "subtitle_temp");
	if (ossAddr == "" || subtitle_temp == "")
	{
		LogManager::instance().log(LogManager::LV_ERROR, "", R"([Rendworker Error]Before work init failed : can not get "ossAddr" or "subtitle_temp")");
		ThreadManager::instanse().stop();
	}
	string tempNoMarkPath = localUpdateDirPath + "CombinedVideo_noMartk.mp4";
	string tempMarkedPath = localUpdateDirPath + "CombinedVideo_marked.mp4";
	string marked_addr = "";

	string noMark_addr = "";
	vector<string> vidFilePaths;
	vector<thread> vec_beautyThreads;
	int index = 0;
	string subVidPath = localUpdateDirPath + "subed.mp4";
	string combinedVid = localUpdateDirPath + "CombinedVideo.mp4";
	string audioedVid = localUpdateDirPath + "VidWithAudio.mp4";
	string finalVid_noMark = localUpdateDirPath + "FinalVideo_noMark.mp4";
	string finalVid_mark = localUpdateDirPath + "FinalVideo_marked.mp4";
	char logBuf[10000] = { 0 };
	while (_runFlag)
	{
		cout << "Geting task..." << endl;
		if (NetCom::instanse().getMission(data))
		{
			memset(logBuf, 0, sizeof(logBuf));
			taskFlag = true;
			time_t start_time = time(NULL);
			cout << "[rendworker] : Got task.[timestamp : " << start_time << "]" << endl;
			NetCom::instanse().sendState(data.uid, NetCom::stateType::started);

			if (!ConfigPhaser::getInstance()->phase(localTemplatesDirPath, data.contents.templateSrc))
			{
				LogManager::instance().log(LogManager::LV_ERROR, data.contents.sessionid,
					"[Rendworker Error]Can not parse template \"%s\"", data.contents.templateSrc.c_str());
				NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Template phase error");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}
			configInfo = ConfigPhaser::getInstance()->getConfig();

			vector<Video> vec_userVids;
			sort(configInfo->videoInfo.videos.begin(), configInfo->videoInfo.videos.end(), \
				[=](const Video& vInfoA, const Video& vInfoB) {return vInfoA.pos < vInfoB.pos; });
			for (auto vidInfo : configInfo->videoInfo.videos)
			{
				if (vidInfo.type == "user")
				{
					vec_userVids.push_back(vidInfo);
				}
			}
			if (data.contents.assets.vec_vidInfos.size() != vec_userVids.size())
			{
				LogManager::instance().log(LogManager::LV_ERROR, data.contents.sessionid,
					"[Rendworker Error]Template \"%s\" is not match, current user video count is %d", data.contents.templateSrc.c_str(), data.contents.assets.vec_vidInfos.size());
				NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Template is not match");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				continue;
			}
			//AliSDKHandle::GetInstance()->AliClientInit(data.contents.ossinfos.region + ".aliyuncs.com", data.contents.ossinfos.access_key_id, \
            //    data.contents.ossinfos.access_key_secret);
			AliSDKHandle::GetInstance()->AliClientInit(data.contents.ossinfos.region + ossAddr, data.contents.ossinfos.access_key_id,
				data.contents.ossinfos.access_key_secret);
			//Downlad sources
			vidFilePaths.clear();
			vec_beautyThreads.clear();
			index = 0;
			tempNoMarkPath = localUpdateDirPath + "CombinedVideo_noMartk.mp4";
			tempMarkedPath = localUpdateDirPath + "CombinedVideo_marked.mp4";

			for (auto vidInfo : data.contents.assets.vec_vidInfos)
			{
				userTaskParam param;
				param.vidInfo = vidInfo;
				param.vid = vec_userVids.at(index);
				param.configInfo = configInfo;
				param.data = data;

				param.localUpdateDirPath = localUpdateDirPath;
				ThreadManager::instanse().addUsrVidTask(param);
				++index;
			}
			ThreadManager::instanse().waitForTasksEnd();
			if (!taskFlag)
			{
				goto THREAD_FINALHANDLE;
			}
#if 0
			//叠加字幕
			if (configInfo->assInfo.location != "")
			{
				cout << "[rendworker] : Adding subtitle...[timestamp : " << time(NULL) << "]" << endl;
				string outPut = localUpdateDirPath;
				if (!VideoHandler::instanse().addSubtitle(configInfo->videoInfo.videos[configInfo->assInfo.pos - 1].location, configInfo->assInfo.location, \
					data.contents.assets.assContent.value, outPut, "subed_noMark.mp4"))
				{
					LogManager::instance().log(LogManager::LV_ERROR, data.contents.sessionid, "Failed to add subtitle[noMark]");
					NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Failed to add subtitle[noMark]");
					goto THREAD_FINALHANDLE;
				}
				if (!VideoHandler::instanse().addSubtitle(configInfo->videoInfo.videos[configInfo->assInfo.pos - 1].markedFile_location, configInfo->assInfo.location, \
					data.contents.assets.assContent.value, outPut, "subed_marked.mp4"))
				{
					LogManager::instance().log(LogManager::LV_ERROR, data.contents.sessionid, "Failed to add subtitle[mark]");
					NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Failed to add subtitle[mark]");
					goto THREAD_FINALHANDLE;
				}
				configInfo->videoInfo.videos[configInfo->assInfo.pos - 1].location = outPut + "subed_noMark.mp4";
				configInfo->videoInfo.videos[configInfo->assInfo.pos - 1].markedFile_location = outPut + "subed_marked.mp4";
				cout << "[rendworker] : Subtitle added! [timestamp : " << time(NULL) << "]" << endl;
			}
#endif
			//叠加字幕
			if (0 < configInfo->assInfo.pos)
			{
				cout << "[rendworker] : Adding subtitle...[timestamp : " << time(NULL) << "]" << endl;

				if (!VideoHandler::instanse().addSubtitle(configInfo->videoInfo.videos[configInfo->assInfo.pos - 1].location, configInfo->assInfo.location, \
					data.contents.assets.assContent.value, localUpdateDirPath, subVidPath))
				{
					LogManager::instance().log(LogManager::LV_ERROR, data.contents.sessionid, "[Rendworker Error]Failed to add subtitle!");
					// NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Failed to add subtitle[noMark]");
					taskFlag = false;
					goto THREAD_FINALHANDLE;
				}
				configInfo->videoInfo.videos[configInfo->assInfo.pos - 1].location = subVidPath;
				cout << "[rendworker] : Subtitle added! [timestamp : " << time(NULL) << "]" << endl;
			}

			for (auto vidInfo : configInfo->videoInfo.videos)
			{
				vidFilePaths.push_back(vidInfo.location);
			}
			for (auto it = vidFilePaths.begin(); it != vidFilePaths.end(); ++it)
			{
				string fileName = (*it).substr(0, (*it).find(".mp4"));
				if (!VideoHandler::instanse().mp4Tots(*it, fileName + ".ts"))
				{
					cout << "error : error step : 'mp4Tots_mark\'!" << endl;
					LogManager::instance().log(LogManager::LV_ERROR, data.contents.sessionid, "[Rendworker Error]Failed to trans vid to ts");
					//  NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Failed to combine videos[noMark]");
					taskFlag = false;
					goto THREAD_FINALHANDLE;
				}
				(*it) = fileName + ".ts";
			}
			cout << "[rendworker] : Combining vids...[timestamp : " << time(NULL) << "]" << endl;
			if (!VideoHandler::instanse().combineVids(localUpdateDirPath, vidFilePaths, combinedVid))
			{
				cout << "error : error step : 'combineVids\'!" << endl;
				LogManager::instance().log(LogManager::LV_ERROR, data.contents.sessionid, "[Rendworker Error]Failed to combine videos");
				//  NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Failed to combine videos[noMark]");
				taskFlag = false;
				goto THREAD_FINALHANDLE;
			}
			cout << "[rendworker] : Combine sucessed! [timestamp : " << time(NULL) << "]" << endl;
			{
				finalTaskParams ftps;
				ftps.audioParams.vidPath = combinedVid;
				ftps.audioParams.outFolder = localUpdateDirPath;
				ftps.audioParams.audioPath = configInfo->audioLocation;
				ftps.sessionid = data.contents.sessionid;
				for (auto postInfos : data.contents.vec_postrenders)
				{
					ftps.postParams = postInfos;
					string localFilePath = "";
					if (postInfos.input == "0")
					{
						ftps.watermark_location = "";
						ThreadManager::instanse().addFinalTask(ftps);
						noMark_addr = postInfos.output;
					}
					else if (postInfos.input == "1")
					{
						ftps.watermark_location = configInfo->waterMarkLocation;
						ThreadManager::instanse().addFinalTask(ftps);
						marked_addr = postInfos.output;
					}
				}
				ThreadManager::instanse().waitForTasksEnd();
				if (!taskFlag)
				{
					goto THREAD_FINALHANDLE;
				}
			}

#if 0
			if (!VideoHandler::instanse().waterMark(localUpdateDirPath + "FinalVideo.mp4", configInfo->logoInfo.location, localUpdateDirPath))
			{
				cout << "error : error step : 'waterMark\'!" << endl;
				NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "Failed to add audio");
				goto THREAD_FINALHANDLE;
			}
#endif
			{
			THREAD_FINALHANDLE:
				if (taskFlag)
				{
					NetCom::instanse().sendState(data.uid, NetCom::stateType::finished);
					time_t end_time = time(NULL);
					cout << "[rendworker] : Task all complete! [timestamp : " << end_time << ", total working time costs : " << end_time - start_time << " seconds.]" << endl;
					// int Len = vsnprintf(NULL, 0, format, ap); 
					sprintf(logBuf, "\n-----Rendworker Information:-----\ntemplate name: %s\nsessionid: %s\n\
start time: %I64d\nend time: %I64d\ntotal cost: %I64d seconds\nvideo_marked_address: %s\nvideo_nomark_address: %s\n---------------------------------",
data.contents.templateSrc.c_str(), data.contents.sessionid.c_str(), start_time, end_time, end_time - start_time, marked_addr.c_str(), noMark_addr.c_str());
					LogManager::instance().log(LogManager::LV_INFO, data.contents.sessionid, logBuf);
				}
				else
				{
					NetCom::instanse().sendState(data.uid, NetCom::stateType::error, "task failed!");
				}
				AliSDKHandle::GetInstance()->AliClientClose();
				//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
		}
		else
		{
			cout << "No task." << endl << endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	cout << "Work ended..." << endl;
}

void ThreadManager::stop()
{
	_runFlag = false;
}

void ThreadManager::suspendMe()
{
	_thread->join();
}

void ThreadManager::close()
{
	_runFlag = false;
	if (_thread != nullptr && _thread->joinable())
	{
		cout << "Threads exiting..." << endl;
		_thread->join();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	if (nullptr != _thread)
	{
		delete _thread;
		_thread = nullptr;
	}
	NetCom::instanse().close();
	VideoHandler::instanse().close();
	LogManager::instance().close();
	//Py_Finalize();
}

ThreadManager& ThreadManager::instanse()
{
	static ThreadManager tM;
	return tM;
}