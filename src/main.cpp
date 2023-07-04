#include <cassert>
#include <string>

#include "base/file.h"
#include "base/yaml.h"

using std::string;
using namespace xtreaming;

int main() {
    string filename = "2b.yaml";
    string text;
    if (!ReadFile(filename, &text)) {
        fprintf(stderr, "File read failed: %s.\n", filename.c_str());
        return 1;
    }

    json obj;
    string error;
    if (!ParseYAML(text, &obj, &error)) {
        fprintf(stderr, "%s\n", error.c_str());
        return 2;
    }

    printf("%s\n", obj.dump(4).c_str());
    return 0;
}
