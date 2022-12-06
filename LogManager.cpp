#include "LogManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "common.h"
#include <iostream>
#include "GELF.hpp"
#include "confmaner.h"
using std::cout;
using std::endl;
bool LogManager::init(const char* logFilePath)
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%t][%^%L%$][%@,%!] %v");

    //auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFilePath, 23, 59);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%t][%^%L%$][%@,%!] %v");

    spdlog::sinks_init_list sink_list = { file_sink, console_sink };
	
	
    m_pLogger = new spdlog::logger("log", sink_list.begin(), sink_list.end());
    m_pLogger->set_level(spdlog::level::trace);
	m_pLogger->flush_on(spdlog::level::info);
	is_init = true;
	m_logDirPath = logFilePath;
	m_logDirPath = m_logDirPath.substr(0, m_logDirPath.find_last_of("\\")) + "\\";
	m_appName = ConfRuner::instance().getKeyValue("rendworker", "app_name");
	m_gelfAddr = ConfRuner::instance().getKeyValue("logSvr", "address");
	m_gelfPort = stoi(ConfRuner::instance().getKeyValue("logSvr", "port"));
	if (false == gelf::initialize())
	{
		cout << "Gelf init failed with address\"" << m_gelfAddr << "\" and port \"" << m_gelfPort << "\"!!!" << endl;
		return false;
	}
	gelf::configure(m_gelfAddr, m_gelfPort);
	return true;
}

void LogManager::close()
{
	if (!is_init) return;
	spd::drop_all();
	if (m_pLogger != nullptr)
	{
		delete m_pLogger;
		m_pLogger = nullptr;
	}
	is_init = false;
	gelf::destroy();
}

void LogManager::copyErrorFile(const string& errorSrcPath, const string& suffix)
{
	struct tm* newtime;
	char tmpbuf[128];
	time_t lt;
	time(&lt);
	newtime = localtime(&lt);
	strftime(tmpbuf, 128, "%F-%H_%M_%S", newtime);
	string cmd = "copy " + errorSrcPath + " " + m_logDirPath + tmpbuf + suffix;
	system(cmd.c_str());
	//cout << cmd << "<--------------res::" << res << endl;
}

void LogManager::log(int logLevel, const string& sessionid, const char* format, ...)
{
	if (!is_init)
	{
		cout << "[rendworker] : Logmanager is not initialized, please initialize it before use it!!!" << endl;
		return;
	}
    char logBuf[10000] = { 0 };
    va_list ap;
    va_start(ap, format);
    // int Len = vsnprintf(NULL, 0, format, ap); 
    vsprintf(logBuf, format, ap);
    va_end(ap);
	string tempField = "app_error";
	string tempValue = "process_error";
	if (sessionid != "")
	{
		tempField = "sessionid";
		tempValue = sessionid;
	}
	bool gelfPostRes = false;
	switch (logLevel)
	{
	case LV_INFO:
	{
		m_pLogger->info(logBuf);
		gelf::MessageBuilder message(gelf::Severity::Informational, logBuf);
		message.additionalField("app_name", m_appName.c_str());
		message.additionalField(tempField, tempValue);
		gelfPostRes = gelf::post(message.build());
	}
	break;
	case LV_DEBUG:
	{
		m_pLogger->debug(logBuf);
		gelf::MessageBuilder message(gelf::Severity::Debug, logBuf);
		message.additionalField("app_name", m_appName.c_str());
		message.additionalField(tempField, tempValue);
		gelfPostRes = gelf::post(message.build());
	}
	break;
	case LV_ERROR:
	{
		m_pLogger->error(logBuf);
		gelf::MessageBuilder message(gelf::Severity::Error, logBuf);
		message.additionalField("app_name", m_appName.c_str());
		message.additionalField(tempField, tempValue);
		gelfPostRes = gelf::post(message.build());
	}
	break;
	case LV_WARNING:
	{
		m_pLogger->warn(logBuf);
		gelf::MessageBuilder message(gelf::Severity::Warning, logBuf);
		message.additionalField("app_name", m_appName.c_str());
		message.additionalField(tempField, tempValue);
		gelfPostRes = gelf::post(message.build());
	}
	break;
	case LV_CRITICAL:
	{
		m_pLogger->critical(logBuf);
		gelf::MessageBuilder message(gelf::Severity::Critical, logBuf);
		message.additionalField("app_name", m_appName.c_str());
		message.additionalField(tempField, tempValue);
		gelfPostRes = gelf::post(message.build());
	}
	break;
	default:
	{
		cout << "[rendworker] : unknown logLevel \'" << logLevel << "\'!!" << endl;
		return;
	}
	}
	if (!gelfPostRes)
	{
		m_pLogger->error("[Gelf Error] : failed to post...");
	}
}
