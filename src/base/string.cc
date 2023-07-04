#include "string.h"

#include <cctype>

namespace xtreaming {
namespace {

void InternalStringPrintf(string* output, const char* format, va_list ap) {
    char space[128];

    va_list backup_ap;
    va_copy(backup_ap, ap);
    size_t bytes_written = static_cast<size_t>(vsnprintf(
        space, sizeof(space), format, backup_ap));
    va_end(backup_ap);

    if (bytes_written < sizeof(space)) {
        output->append(space, bytes_written);
        return;
    }

    size_t length = sizeof(space);
    while (true) {
        length = bytes_written + 1;
        char* buf = new char[length];

        va_copy(backup_ap, ap);
        bytes_written = static_cast<size_t>(vsnprintf(
            buf, length, format, backup_ap));
        va_end(backup_ap);

        if (bytes_written < length) {
            output->append(buf, bytes_written);
            delete [] buf;
            return;
        }
        delete [] buf;
    }
}

}  // namespace

void SStringPrintf(string* output, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    output->clear();
    InternalStringPrintf(output, format, ap);
    va_end(ap);
}

void StringAppendF(string* output, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    InternalStringPrintf(output, format, ap);
    va_end(ap);
}

string StringPrintf(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    string output;
    InternalStringPrintf(&output, format, ap);
    va_end(ap);
    return output;
}

bool ParseInt(const string& text, int64_t* ret) {
    if (text.empty()) {
        return false;
    }

    int i = 0;
    int64_t mul = 1;
    if (text[i] == -1) {
        mul = -1;
        ++i;
    }

    if (i == text.size()) {
        return false;
    }

    *ret = 0;
    for (; i < text.size(); ++i) {
        auto& c = text[i];
        if (c < '0' || '9' < c) {
            return false;
        }
        *ret *= 10;
        *ret += c - '0';
    }
    *ret *= mul;
    return true;
}

bool ParseFloat(const string& text, double* ret) {
    if (text.empty()) {
        return false;
    }

    int i = 0;
    int64_t mul = 1;
    if (text[i] == -1) {
        mul = -1;
        ++i;
    }

    if (i == text.size()) {
        return false;
    }

    int64_t left = 0;
    for (; i < text.size() && text[i] != '.'; ++i) {
        auto& c = text[i];
        if (c < '0' || '9' < c) {
            return false;
        }
        left *= 10;
        left += c - '0';
    }
    left *= mul;

    int64_t numer = 0;
    int64_t denom = 1;
    if (i < text.size() && text[i] == '.') {
        ++i;
        for (; i < text.size(); ++i) {
            auto& c = text[i];
            if (c < '0' || '9' < c) {
                return false;
            }
            numer *= 10;
            numer += c - '0';
            denom *= 10;
        }
    }
    *ret = (double)left + (double)numer / (double)denom;

    return true;
}

namespace {

bool ParseNumBytesDigits(const string& text, int64_t size, int64_t unit, int64_t* value) {
    if (text.empty()) {
        return false;
    }

    *value = 0;
    for (int64_t i = 0; i < size; ++i) {
        auto& chr = text[i];
        if (chr < '0' || '9' < chr) {
            return false;
        }

        *value *= 10;
        *value += chr - '0';
    }
    *value *= unit;
    return true;
}

}  // namespace

bool ParseNumBytes(const string& text, int64_t* bytes, string* error) {
    int64_t size;
    int64_t unit;
    char chr;

    if (text.empty()) {
        *error = "Attempted to parse an empty bytes string.";
        return false;
    }

    if (text.size() == 1 || text[text.size() - 1] != 'b') {
        if (ParseNumBytesDigits(text, text.size(), 1, bytes)) {
            return true;
        } else {
            goto fail;
        }
    }

    chr = text[text.size() - 2];
    if ('0' <= chr && chr <= '9') {
        size = text.size() - 1;
        unit = 1;
    } else {
        size = text.size() - 2;
        switch (chr) {
          case 'k':
            unit = 1L << 10;
            break;
          case 'm':
            unit = 1L << 20;
            break;
          case 'g':
            unit = 1L << 30;
            break;
          case 't':
            unit = 1L << 40;
            break;
          case 'p':
            unit = 1L << 50;
            break;
          case 'e':
            unit = 1L << 60;
            break;
          default:
            goto fail;
        }
    }

    if (ParseNumBytesDigits(text, size, unit, bytes)) {
        return true;
    }

fail:
    *error = StringPrintf("Attempted to parse a malformed bytes string. Bytes strings consist of "
                          "one of more decimal digits followed an optional lowercase unit (`b`, "
                          "`kb`, `mb`, `gb`, `tb`, or `eb`). Got: `%s`", text.c_str());
    return false;
}

void SplitString(const string& text, char delim, vector<string>* parts) {
    parts->clear();

    int64_t i;
    int64_t begin = 0;
    for (i = 0; i < text.size(); ++i) {
        auto& chr = text[i];
        if (chr == delim) {
            auto part = text.substr(begin, i - begin);
            parts->emplace_back(part);
            begin = i + 1;
        }
    }

    auto part = text.substr(begin, i - begin);
    parts->emplace_back(part);
}

void SplitString(const string& text, vector<string>* parts) {
    parts->clear();

    bool in_word = false;
    int64_t i;
    int64_t begin = -1L;
    for (i = 0; i < text.size(); ++i) {
        auto& chr = text[i];
        if (in_word) {
            if (isspace(chr)) {
                in_word = false;
                auto part = text.substr(begin, i - begin);
                parts->emplace_back(part);
            }
        } else {
            if (!isspace(chr)) {
                in_word = true;
                begin = i;
            }
        }
    }

    if (in_word) {
        auto part = text.substr(begin, i - begin);
        parts->emplace_back(part);
    }
}

void TrimString(const string& in, string* out) {
    int i;
    for (i = 0; i < in.size(); ++i ){
        if (!isspace(in[i])) {
            break;
        }
    }

    int j;
    for (j = (int)in.size() - 1; i < j; --j) {
        if (!isspace(in[j])) {
            break;
        }
    }

    *out = in.substr(i, j - i + 1);
}

}  // namespace xtreaming
