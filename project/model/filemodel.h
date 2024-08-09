//Created by mars on 9/8/24

#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <string>
#include <vector>
#include "file.h"

class FileModel {
public:
    void insertFile(int sender_id, int receiver_id, std::string file_name);
    
    std::vector<File> query(int receiver_id);
};

#endif //FILEMODEL_H
