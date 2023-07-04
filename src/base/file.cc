#include "file.h"

#include <cstdio>

namespace xtreaming {

bool ReadFile(const string& filename, string* data) {
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    data->clear();
    data->resize(size);
    fread(&(*data)[0], size, 1, file);
    fclose(file);
    return true;
}

}  // namespace xtreaming
