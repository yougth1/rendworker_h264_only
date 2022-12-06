#include "CloseHandler.h"
#include <iostream>
#include "ThreadManager.h"
bool CloseHandler::closingFlag = false;
bool CloseHandler::Init()
{
	closingFlag = false;
	if (!::SetConsoleCtrlHandler((PHANDLER_ROUTINE)handleEvent, TRUE))
	{
		std::cout << "CloseHandler init failed" << std::endl;
		return false;
	}
	return true;
}

BOOL CloseHandler::handleEvent(DWORD ctrlType)
{
	switch (ctrlType)
	{
	//case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	{
		closingFlag = true;
		std::cout << "Closing, please wait..." << std::endl;
		ThreadManager::instanse().stop();
		return TRUE;
	}
	break;
	default:
		return FALSE;
	}
}

bool CloseHandler::isClosing()
{
	return closingFlag;
}

CloseHandler& CloseHandler::instanse()
{
	static CloseHandler handler;
	return handler;
}
