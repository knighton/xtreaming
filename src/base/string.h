#pragma once

#include <cstdint>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace xtreaming {

// Equivalents of printf that work on strings.
// Clears output before writing to it.
void SStringPrintf(string* output, const char* format, ...);
void StringAppendF(string* output, const char* format, ...);
string StringPrintf(const char* format, ...);

// Parse ints and floats.
bool ParseInt(const string& text, int64_t* ret);
bool ParseFloat(const string& text, double* ret);

// Parse a bytes string (e.g. "7", 7b, 7kb, 7mb, 7gb, 7tb, 7pb, 7eb) to integer.
bool ParseNumBytes(const string& text, int64_t* bytes, string* error);

// Split.
void SplitString(const string& text, char chr, vector<string>* parts);  // By char.
void SplitString(const string& text, vector<string>* parts);            // Python-style.

// Trim.
void TrimString(const string& in, string* out);

}  // namespace xtreaming
