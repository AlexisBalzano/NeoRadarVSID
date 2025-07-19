#pragma once

namespace PluginSDK::Logger {

/**
 * @enum LogLevel
 * @brief Log levels for plugin logging
 */
enum class LogLevel {
    Fatal = 0,
    Error = 1,
    Warning = 2,
    Info = 3,
    Debug = 4,
    Verbose = 5
};

/**
 * @interface LoggerAPI
 * @brief Interface for plugin logging
 */
class LoggerAPI {
public:
    virtual ~LoggerAPI() = default;

    /**
     * @brief Log a message with specified level
     * @param level Log level
     * @param message Message to log
     */
    virtual void log(LogLevel level, const std::string& message) = 0;

    /**
     * @brief Log a fatal message
     * @param message Message to log
     */
    virtual void fatal(const std::string& message) = 0;

    /**
     * @brief Log an error message
     * @param message Message to log
     */
    virtual void error(const std::string& message) = 0;

    /**
     * @brief Log a warning message
     * @param message Message to log
     */
    virtual void warning(const std::string& message) = 0;

    /**
     * @brief Log an info message
     * @param message Message to log
     */
    virtual void info(const std::string& message) = 0;

    /**
     * @brief Log a debug message
     * @param message Message to log
     */
    virtual void debug(const std::string& message) = 0;

    /**
     * @brief Log a verbose message
     * @param message Message to log
     */
    virtual void verbose(const std::string& message) = 0;
};

} // namespace PluginSDK
