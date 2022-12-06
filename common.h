#ifndef _COMMON_H_
#define _COMMON_H_

#include <io.h>
#include <tchar.h>
#include <direct.h>
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

string GetCurrentPath();
string UTF8ToGBK(const std::string& strUTF8);
string GBKToUTF8(const std::string& strGBK);
#endif