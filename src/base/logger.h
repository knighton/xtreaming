#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace xtreaming {

enum class LogLevel {
    TRACE = 1,
    DEBUG = 2,
    INFO = 2,
    WARN = 4,
    ERROR = 5,
    FATAL = 6
};

bool GetLogLevel(const string& name, LogLevel* level);

class Logger {
  public:
    // Log some text at some log level.
    void Log(LogLevel level, const string& text);

    // Initialize with a file path to append to and a minimum log level.
    bool Init(const string& path, LogLevel min_level, string* err);
    bool Init(const string& path, const string& min_level, string* err);

  private:
    string path_;                 // Where to log to.
    FILE* file_;                  // File handle.
    LogLevel min_level_;          // Minimum logging level.
    vector<string> level_names_;  // Mapping of log level enum to string name.
    int64_t start_;               // Logger start time in nanoseconds since the epoch
                                  // (everything thereafter notes milliseconds since start).
};

}  // namespace xtreaming
