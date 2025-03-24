#ifndef file_module_h
#define file_module_h

#include "SPIFFS.h"

class FileRW
{
public:
    bool init();
    void writeFile(fs::FS &fs, String filename, String message);
    String readFile(fs::FS &fs, String filename);
};

#endif