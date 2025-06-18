#pragma once
#include <iostream>
#include <vector>
#include <string.h>

#include "dhnetsdk.h"
#include <grpcpp/grpcpp.h>
#include "MonitorWall.grpc.pb.h"
using grpc::Status;

#define REALPLAY	//控制实时播放功能

#define MAX_BUF_LEN (2 * 1024 * 1024)	//获取配置的缓冲区大小
#define WAIT_TIME			5000
#define SWKJ_BLOCK_SIZE         8192

extern std::vector<int> vwinID;     //存放窗口ID，用于关窗操作
extern std::vector<DH_IN_SPLIT_OPEN_WINDOW> vecOpenInputPara;
extern std::vector<DH_OUT_SPLIT_OPEN_WINDOW> vecOpenOutputPara;
extern int globalCode;

//用于绑定输出口，所有值都是从0开始
typedef struct BindOutputTV{
    int BindLine;   //block横坐标
    int BindCol;    //block纵坐标
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
}decoderInfo;

typedef struct wallConfig{      //显示使用
    int TVID;
    int b_line;
    int b_col;
}wallConfig;

//绑定的信号源
typedef struct bindSourceInfo{
    char sourceIP[16];
	// WORD sourcePort;
	char sourceName[16];
	char sourcePwd[16];
    int sourceChaneel;
}bindSourceInfo;

//存储错误信息
extern std::map<int, std::string> codeMap;

//编码方式
extern std::map<int, std::string> encoderCompression;

//分辨率
extern std::map<int, std::string> resolution;

//用于保存已绑定的信号源
extern std::vector<bindSourceInfo> vecStuBindSource;

extern std::vector<int> vwinID;     //存放窗口ID，用于关窗操作
extern std::vector<decoderInfo> vecDecoderInfo;   //存放已经登录的解码器信息
extern MonitorWall::allWindowDisplayMesResponse* all_mes;		//显示信息


void SaveJsonToFile(const std::string& filename, const char* jsonData);    //保存文件到Json
bool IsLogin(); //判断是否登录
void saveLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd);   //保存登录信息
bool queryLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd);  //查询登录信息

// 清空vector
void clearVector(std::vector<DH_IN_SPLIT_OPEN_WINDOW>& vecInput, std::vector<DH_OUT_SPLIT_OPEN_WINDOW>& vecOutput);

// 2分屏
void openWindowsTwo(int Chn);
// 根号分屏
void openWindowSqrt(int Chn, int openNum);
//特殊分屏
void openWindowspecial(int Chn, int openNum);

Status codeProccess(MonitorWall::codeResponse* request, MonitorWall::empty* response);