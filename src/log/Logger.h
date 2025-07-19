#pragma once

#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "SDK.h"

namespace vsid::logging {
class Logger {
   public:
    enum LogSender {
        vACDM,
        DataManager,
        Server,
        ConfigParser,
        Utils,
    };

    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Critical,
        System,
        Disabled,
    };

    struct LogSetting {
        LogSender sender;
        std::string name;
        LogLevel minimumLevel;
    };

    struct AsynchronousLog {
        LogSender sender;
        std::string message;
        LogLevel loglevel;
    };

   private:
    Logger();
#ifdef DEBUG_BUILD
    std::vector<LogSetting> logSettings = {
        {vACDM, "vACDM", Debug},   {DataManager, "DataManager", Info},
        {Server, "Server", Debug}, {ConfigParser, "ConfigParser", Debug},
        {Utils, "Utils", Debug},
    };
#else
    /// @brief set the log level for each sender separately
    std::vector<LogSetting> logSettings = {
        {vACDM, "vACDM", Disabled},   {DataManager, "DataManager", Disabled},
        {Server, "Server", Disabled}, {ConfigParser, "ConfigParser", Disabled},
        {Utils, "Utils", Disabled},
    };
#endif
    bool m_LogAll = false;

    std::mutex m_logLock;
    std::list<struct AsynchronousLog> m_asynchronousLogs;
    std::thread m_logWriter;
    bool m_stop = false;
    void run();

    void enableLogging();
    void disableLogging();
    bool loggingEnabled = false;

    std::stringstream stream;
    bool logFileCreated = false;

    PluginSDK::Logger::LoggerAPI *vacdmLogger_ = nullptr;;

   public:
    ~Logger();
    /// @brief queues a log message to be processed asynchronously
    /// @param sender the sender (e.g. class)
    /// @param message the message to be displayed
    /// @param loglevel the severity, must be greater than m_minimumLogLevel to be logged
    void log(const LogSender &sender, const std::string &message, const LogLevel loglevel);
    void setLogger(PluginSDK::Logger::LoggerAPI *vacdmLogger) { vacdmLogger_ = vacdmLogger; };
    std::string handleLogCommand(std::string command);
    std::string handleLogLevelCommand(std::string command);
    static Logger &instance();
};
}  // namespace vacdm::logging