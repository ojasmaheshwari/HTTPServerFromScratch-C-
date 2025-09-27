#pragma once

#include <string>

enum class LoggingLevel {
  LogLevelInfo = 0, LogLevelWarning, LogLevelError
};

class Logging {
public:
  Logging();
  ~Logging();
  Logging(LoggingLevel loggingLevel, const std::string &className);

  LoggingLevel getLoggingLevel() const;
	std::string getClassName() const;

  void setLoggingLevel(LoggingLevel loggingLevel);
	void setClassName(const std::string& className);

  void log(const std::string &message);
  void log(const std::string &message, LoggingLevel loggingLevel);
  void info(const std::string &message);
  void warn(const std::string &message);
  void error(const std::string &message);
private:
  LoggingLevel m_LoggingLevel;
  std::string m_ClassName;

  std::string get_current_time();
};