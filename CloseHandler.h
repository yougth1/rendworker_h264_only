#pragma once
#include <windows.h>
class CloseHandler final
{
public:
	bool Init();
	static BOOL handleEvent(DWORD ctrlType);
	bool isClosing();

	static CloseHandler& instanse();
private:
	CloseHandler() {};
	CloseHandler(CloseHandler&);
	CloseHandler& operator=(CloseHandler&) {};
	static bool closingFlag;
}; 