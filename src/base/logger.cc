#include "logger.h"

namespace xtreaming {

bool GetLogLevel(const string& name, LogLevel* level) {
    if (name == "trace") {
        *level = LogLevel::TRACE;
    } else if (name == "debug") {
        *level = LogLevel::DEBUG;
    } else if (name == "info") {
        *level = LogLevel::INFO;
    } else if (name == "warn") {
        *level = LogLevel::WARN;
    } else if (name == "error") {
        *level = LogLevel::ERROR;
    } else if (name == "fatal") {
        *level = LogLevel::FATAL;
    } else {
        return false;
    }
    return true;
}

}  // namespace xtreaming
