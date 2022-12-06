#pragma once
#include <thread>
#include <queue>
#include "ConfigPhaser.h"
#include "Agreenments.h"
struct userTaskParam
{
	string localUpdateDirPath;
	vedioInfo vidInfo;
	missionData data;
	Video vid;
	ConfigInfo* configInfo;
};

class ThreadManager final
{
public:
	bool init();
	void start();
	void waitForTasksEnd();
	void addUsrVidTask(userTaskParam param);
	static void usrVidTask(userTaskParam param);
	static void finalTask(finalTaskParams ftps);
	void addFinalTask(finalTaskParams ftps);
	void stop();
	void suspendMe();
	void close();
	static ThreadManager& instanse();
private:

	void startWork();
	ThreadManager() : _thread(nullptr){};
	ThreadManager(ThreadManager&);
	ThreadManager& operator=(ThreadManager&) {};	

	static void handleEvent();
private:
	thread* _thread;
	static bool _runFlag;
	vector<thread*> m_vec_tasks;
};

