#include <fstream>
#include "Utils.h"
#include <vector>
#include <string>

std::vector<decoderInfo> vecDecoderInfo;

void SaveJsonToFile(const std::string& filename, const char* jsonData)
{
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << jsonData;
        outFile.close();
        std::cout << "JSON data save to file " << filename << std::endl;
    } else {
        std::cerr << "cannot open " << filename << std::endl;
    }
}

void saveLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd)
{
    decoderInfo gstuSaveInfo(ip.c_str(), port, user.c_str(), pwd.c_str());
    vecDecoderInfo.emplace_back(gstuSaveInfo);
}

bool queryLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd)
{
    decoderInfo gstuQueryInfo(ip.c_str(), port, user.c_str(), pwd.c_str());

    if(!vecDecoderInfo.empty()){
        for(int i = 0; i < vecDecoderInfo.size(); i++){
            if(vecDecoderInfo[i] == gstuQueryInfo)
            {
                return true;    //登录成功
            }
        }
        return false;   //未登录
    }else{
        return false;   //未登录
    }
}