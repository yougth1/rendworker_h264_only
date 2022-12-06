#include <Windows.h>
#include <iostream>
#include "CloseHandler.h"
#include "ThreadManager.h"

using namespace std;

int main(void)
{
	if (!CloseHandler::instanse().Init() || !ThreadManager::instanse().init())
	{
		cout << "error : init failed!" << endl;
		return -1;
	}
	do {
		ThreadManager::instanse().start();
		ThreadManager::instanse().suspendMe();
	} while (!CloseHandler::instanse().isClosing());
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ThreadManager::instanse().close();
	return 0;
}