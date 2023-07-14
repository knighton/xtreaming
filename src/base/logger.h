#pragma once

#include <string>

using std::string;

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

}  // namespace xtreaming
