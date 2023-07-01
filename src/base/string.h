#pragma once

#include <string>

using std::string;

namespace streaming {

// Equivalents of printf that work on strings.
// Clears output before writing to it.
void SStringPrintf(string* output, const char* format, ...);
void StringAppendF(string* output, const char* format, ...);
string StringPrintf(const char* format, ...);

// Parse a bytes string (e.g. "7", 7b, 7kb, 7mb, 7gb, 7tb, 7pb, 7eb) to integer.
bool ParseNumBytes(const string& text, int64_t* bytes, string* error);

}  // namespace streaming
