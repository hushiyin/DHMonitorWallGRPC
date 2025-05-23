#pragma once
#include <iostream>
#include <vector>
#include <string.h>

#include "dhnetsdk.h"

#define REALPLAY	//控制实时播放功能
// #define IS_IMPL		//已经实现的功能

#define MAX_BUF_LEN (2 * 1024 * 1024)	//获取配置的缓冲区大小
#define WAIT_TIME			5000

//用于绑定输出口，所有值都是从0开始
typedef struct BindOutputTV{
    int BindLine;   //block行
    int BindCol;   //block列
    int BindCh;     //需要绑定的输出口
};

typedef struct decoderInfo{
    char decoderIp[64];
	WORD decoderPort;					// tcp 连接端口，需与期望登录设备页面 tcp 端口配置一致
	char decoderName[64];
	char decoderPwd[64];

    //构造函数
    decoderInfo(const char* ip, WORD port, const char* user, const char* pwd)
    {
        strcpy(decoderIp, ip);
        strcpy(decoderName, user);
        strcpy(decoderPwd, pwd);
        decoderPort = port;
    }

    //重载“==”
    bool operator==(const decoderInfo& other) const {
        return (strcmp(decoderIp, other.decoderIp) == 0) &&
                (decoderPort == other.decoderPort) &&
                (strcmp(decoderName, other.decoderName) == 0) &&
                (strcmp(decoderPwd, other.decoderPwd) == 0);
    }
};

extern std::vector<decoderInfo> vecDecoderInfo;   //存放已经登录的解码器信息

void SaveJsonToFile(const std::string& filename, const char* jsonData);    //保存文件到Json
bool IsLogin(); //判断是否登录
void saveLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd);   //保存登录信息
bool queryLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd);  //查询登录信息