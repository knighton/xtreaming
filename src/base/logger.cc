#include "logger.h"

#include "base/string.h"
#include "base/time.h"

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

void Logger::Log(LogLevel level, const string& text) {
    if (level < min_level_) {
        return;
    }

    double time = (double)(NanoTime() - start_) / 1e6;
    auto& level_name = level_names_[(size_t)level];
    string line = StringPrintf("%14.3f [%s] %s\n", time, level_name.c_str(), text.c_str());
    fputs(line.c_str(), file_);
    fflush(file_);
}

bool Logger::Init(const string& path, LogLevel min_level, string* err) {
    path_ = path;
    file_ = fopen(path.c_str(), "ab");
    if (!file_) {
        return false;
    }

    min_level_ = min_level;
    level_names_ = {
        "",
        "trace",
        "debug",
        "info",
        "warn",
        "error",
        "fatal"
    };
    start_ = NanoTime();
    string line = StringPrintf("[start] unix time %.9f", (double)start_ / 1e9);
    Log(LogLevel::INFO, line);
    return true;
}

bool Logger::Init(const string& path, const string& min_level_name, string* err) {
    LogLevel min_level;
    if (!GetLogLevel(min_level_name, &min_level)) {
        *err = StringPrintf("Unknown log level (must be `trace`, `debug`, `info`, `warn`, "
                            "`error`, or `fatal`), but got: `%s`.", min_level_name.c_str());
        return false;
    }

    return Init(path, min_level, err);
}

}  // namespace xtreaming
