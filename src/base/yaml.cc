#include "yaml.h"

#include <cstdint>
#include <map>
#include <vector>

#include "base/string.h"

using std::map;
using std::vector;

namespace xtreaming {

namespace {

bool Build(const vector<string>& lines, const vector<vector<int>>& children, int index,
           json* obj, string* error) {
    long colon;
    string key;
    string val;
    int64_t val_i64;
    double val_f64;
    for (auto& child : children[1 + index]) {
        auto& line = lines[child];
        colon = line.find(':');
        if (colon == string::npos) {
            *error = StringPrintf("No `:` found on line %d: `%s`.", child, line.c_str());
            return false;
        }

        TrimString(line.substr(0, colon), &key);
        if (!children[1 + child].empty()) {
            (*obj)[key] = json::object();
            if (!Build(lines, children, child, &(*obj)[key], error)) {
                return false;
            }
        } else {
            TrimString(line.substr(colon + 1), &val);
            (*obj)[key] = val;
            if (val == "null") {
                (*obj)[key] = nullptr;
            } else if (val == "true") {
                (*obj)[key] = true;
            } else if (val == "false") {
                (*obj)[key] = false;
            } else if (ParseInt(val, &val_i64)) {
                (*obj)[key] = val_i64;
            } else if (ParseFloat(val, &val_f64)) {
                (*obj)[key] = val_f64;
            } else {
                (*obj)[key] = val;
            }
        }
    }
    return true;
}

}  // namespace

bool ParseYAML(const string& text, json* obj, string* error) {
    // Get lines.
    vector<string> lines;
    SplitString(text, '\n', &lines);

    // Get indents.
    vector<int> indents;
    indents.reserve(lines.size());
    for (int i = 0; i < lines.size(); ++i) {
        auto& line = lines[i];
        if (line.empty()) {
            indents.emplace_back(-1);
            continue;
        }

        int j = 0;
        while (j < line.size() && line[j] == ' ') {
            ++j;
        }
        if (j == line.size()) {
            continue;
        }
        if (j % 2) {
            *error = StringPrintf("Irregular indent on line %d: `%s`.", i, line.c_str());
            return false;
        }
        indents.emplace_back(j / 2);
    }

    // Get the children of each line.
    map<int, int> state = {{-1, -1}};
    vector<vector<int>> children;
    children.resize(1 + lines.size());
    for (int i = 0; i < lines.size(); ++i) {
        int indent = indents[i];
        if (indent == -1) {
            continue;
        }
        int parent = state[indent - 1];
        children[1 + parent].emplace_back(i);
        state[indent] = i;
    }

    // Recursively build the tree.
    return Build(lines, children, -1, obj, error);
}

}  // namespace xtreaming
