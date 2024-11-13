#include "file_rw.h"

bool FileRW::init()
{
    if (!SPIFFS.begin(true))
    {
        log_e("An error has occurred while mounting SPIFFS");
        return 1;
    }
    log_i("SPIFFS mounted successfully");
    return 1;
}

void FileRW::writeFile(fs::FS &fs, String filename, String message)
{
    log_i("writeFile -> Writing file: %s", filename);

    File file = fs.open(filename, FILE_WRITE);
    if (!file)
    {
        log_i("writeFile -> failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        log_i("writeFile -> file written");
    }
    else
    {
        log_i("writeFile -> write failed");
    }
    file.close();
}

String FileRW::readFile(fs::FS &fs, String filename)
{
    log_i("readFile -> Reading file: %s", filename);

    File file = fs.open(filename);
    if (!file || file.isDirectory())
    {
        log_i("readFile -> failed to open file for reading");
        return "";
    }

    String fileText = "";
    while (file.available())
    {
        fileText = file.readString();
    }

    file.close();
    return fileText;
}
