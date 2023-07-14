#include <cassert>
#include <string>
#include <thread>

#include "base/file.h"
#include "base/yaml.h"
#include "dataset.h"

using namespace std::chrono_literals;
using std::string;
using namespace xtreaming;

int main() {
    string filename = "2b.yaml";

    // Read YAML file to string.
    string txt;
    if (!ReadFile(filename, &txt)) {
        fprintf(stderr, "File read failed: %s.\n", filename.c_str());
        return 1;
    }

    // Parse the YAML.
    json obj;
    string err;
    if (!ParseYAML(txt, &obj, &err)) {
        fprintf(stderr, "%s\n", err.c_str());
        return 2;
    }
    printf("%s\n", obj.dump(4).c_str());

    // Init streaming dataset.
    Dataset dataset;
    if (!dataset.Init(obj, &err)) {
        fprintf(stderr, "%s\n", err.c_str());
        return 3;
    }

    assert(dataset.Iter());

    // Sleep.
    // std::this_thread::sleep_for(60s);

    return 0;
}
