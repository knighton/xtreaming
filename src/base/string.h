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
bool ParseInt(const string& txt, int64_t* ret);
bool ParseInt(const string& text, double* ret);
bool ParseFloat(const string& txt, double* ret);

// Parse bytes (`7kb`), counts (`7k`), and times (`7s`).
bool ParseBytes(const string& txt, int64_t* ret, string* err);
bool ParseCount(const string& txt, int64_t* ret, string* err);
bool ParseTime(const string& txt, double* ret, string* err);

// Split.
void SplitString(const string& txt, char chr, vector<string>* ret);  // By char.
void SplitString(const string& txt, vector<string>* ret);            // Python-style.

// Trim.
void TrimString(const string& in, string* out);

}  // namespace xtreaming
