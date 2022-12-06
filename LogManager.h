#pragma once
#include "singleton.h"
#include "spdlog/spdlog.h"
namespace spd = spdlog;
#include <string>
using std::string;
class LogManager final : public SingleTon<LogManager>
{
public:
	enum logLevel
	{
		LV_DEBUG,
		LV_INFO,
		LV_ERROR,
		LV_WARNING,
		LV_CRITICAL,
		LV_MARK,
		LV_MARKE
	};
public:
	bool init(const char* logFilePath);
	void close();
	void copyErrorFile(const string& errorSrcPath, const string& suffix);
	void log(int logLevel, const string& sessionid, const char* format, ...);
private:
	spdlog::logger* m_pLogger = nullptr;
	bool is_init = false;
	string m_logDirPath;
	string m_appName;
	string m_gelfAddr;
	uint16_t m_gelfPort;
};

