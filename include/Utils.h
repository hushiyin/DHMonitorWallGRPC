#pragma once
#include <iostream>
#include <vector>
#include <string.h>

#include "dhnetsdk.h"
#include <grpcpp/grpcpp.h>
#include "MonitorWall.grpc.pb.h"
using grpc::Status;

#define REALPLAY	//����ʵʱ���Ź���

#define MAX_BUF_LEN (2 * 1024 * 1024)	//��ȡ���õĻ�������С
#define WAIT_TIME			5000
#define SWKJ_BLOCK_SIZE         8192

extern std::vector<int> vwinID;     //��Ŵ���ID�����ڹش�����
extern std::vector<DH_IN_SPLIT_OPEN_WINDOW> vecOpenInputPara;
extern std::vector<DH_OUT_SPLIT_OPEN_WINDOW> vecOpenOutputPara;
extern int globalCode;

//���ڰ�����ڣ�����ֵ���Ǵ�0��ʼ
typedef struct BindOutputTV{
    int BindLine;   //block������
    int BindCol;    //block������
    int BindCh;     //��Ҫ�󶨵������
};

typedef struct decoderInfo{
    char decoderIp[64];
	WORD decoderPort;					// tcp ���Ӷ˿ڣ�����������¼�豸ҳ�� tcp �˿�����һ��
	char decoderName[64];
	char decoderPwd[64];

    //���캯��
    decoderInfo(const char* ip, WORD port, const char* user, const char* pwd)
    {
        strcpy(decoderIp, ip);
        strcpy(decoderName, user);
        strcpy(decoderPwd, pwd);
        decoderPort = port;
    }

    //���ء�==��
    bool operator==(const decoderInfo& other) const {
        return (strcmp(decoderIp, other.decoderIp) == 0) &&
                (decoderPort == other.decoderPort) &&
                (strcmp(decoderName, other.decoderName) == 0) &&
                (strcmp(decoderPwd, other.decoderPwd) == 0);
    }
}decoderInfo;

typedef struct wallConfig{      //��ʾʹ��
    int TVID;
    int b_line;
    int b_col;
}wallConfig;

//�󶨵��ź�Դ
typedef struct bindSourceInfo{
    char sourceIP[16];
	// WORD sourcePort;
	char sourceName[16];
	char sourcePwd[16];
    int sourceChaneel;
}bindSourceInfo;

//�洢������Ϣ
extern std::map<int, std::string> codeMap;

//���뷽ʽ
extern std::map<int, std::string> encoderCompression;

//�ֱ���
extern std::map<int, std::string> resolution;

//���ڱ����Ѱ󶨵��ź�Դ
extern std::vector<bindSourceInfo> vecStuBindSource;

extern std::vector<int> vwinID;     //��Ŵ���ID�����ڹش�����
extern std::vector<decoderInfo> vecDecoderInfo;   //����Ѿ���¼�Ľ�������Ϣ
extern MonitorWall::allWindowDisplayMesResponse* all_mes;		//��ʾ��Ϣ


void SaveJsonToFile(const std::string& filename, const char* jsonData);    //�����ļ���Json
bool IsLogin(); //�ж��Ƿ��¼
void saveLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd);   //�����¼��Ϣ
bool queryLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd);  //��ѯ��¼��Ϣ

// ���vector
void clearVector(std::vector<DH_IN_SPLIT_OPEN_WINDOW>& vecInput, std::vector<DH_OUT_SPLIT_OPEN_WINDOW>& vecOutput);

// 2����
void openWindowsTwo(int Chn);
// ���ŷ���
void openWindowSqrt(int Chn, int openNum);
//�������
void openWindowspecial(int Chn, int openNum);

Status codeProccess(MonitorWall::codeResponse* request, MonitorWall::empty* response);