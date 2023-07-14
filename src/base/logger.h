#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace xtreaming {

enum class LogLevel {
    TRACE = 1,
    DEBUG = 2,
    INFO = 3,
    WARN = 4,
    ERROR = 5,
    FATAL = 6
};

bool GetLogLevel(const string& name, LogLevel* level);

class Logger;

class ScopeTimer {
  public:
    // Start timing, reporting back to the logger on enter and exit.
    ScopeTimer(const string& name, Logger* logger);

    // Stop timing, reporting back to the logger.
    ~ScopeTimer();

  private:
    string name_;
    Logger* logger_;
    int64_t start_;
};

class Logger {
  public:
    // Log some text at some log level.
    void Log(LogLevel level, const string& text);

    // Initialize with a file path to append to and a minimum log level.
    bool Init(const string& path, LogLevel min_level, string* err);
    bool Init(const string& path, const string& min_level, string* err);

    // Get a scope logger with the given scope name.
    ScopeTimer Scope(const string& name);

  private:
    string path_;                 // Where to log to.
    FILE* file_;                  // File handle.
    LogLevel min_level_;          // Minimum logging level.
    vector<string> level_names_;  // Mapping of log level enum to string name.
    int64_t start_;               // Logger start time in nanoseconds since the epoch
                                  // (everything thereafter notes milliseconds since start).
};

}  // namespace xtreaming
