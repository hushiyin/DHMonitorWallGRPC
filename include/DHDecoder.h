#pragma once
#include <iostream>
#include <string.h>
#include <unistd.h> 
#include <memory>
#include <vector>
#include "CallbackFunc.h"
#include "dhconfigsdk.h"
#include "Utils.h"

#include <grpcpp/grpcpp.h>
#include "MonitorWall.grpc.pb.h"

//������Ϣ
class WindowInfo{
public:
	int m_windowID;			//����ID
	int m_bitRate;			//����
	int m_frameRate;		//֡��
	int m_resolution;		//�ֱ���
	EM_NET_ENCODE_COMPRESSION m_compression;	//���뷽ʽ
	NET_VIDEOCHANNEL_STATE     m_state;     //���ڽ���״̬
};

//����Ϣ
class Block{
public:
	int m_channelID;		//�󶨵�ͨ����
	int m_openWindowNum;	//��������
	char szCompositeID[AV_CFG_Device_ID_Len];	// ����ID

	std::vector<std::shared_ptr<WindowInfo>> m_vwinInfo;	//������Ϣ����
};

class DHDecoder
{
public:
	DHDecoder(const char* ip = "", WORD port = 0, const char* user = "", const char* pwd = "");
	~DHDecoder();
	//��ʼ�����
	void initSDK();
	void getSDKVer();
	void uninitSDK();
	void longinDev();
	void logoutDev();
	int getLastErrorCode();
	int queryDecoderInfo();						//��ѯ��������Ϣ

	//ǽ����
	std::vector<wallConfig> getMonitorWallCfg();					//��ȡ����ǽ����
	void setMonitorWallCfg(int blockNums, std::vector<BindOutputTV>& vec, MonitorWall::tvMesResponse* response);		//��������ǽ
	void setStruct(std::shared_ptr<AV_CFG_MonitorWall> pstuWall, int blockNums, std::vector<BindOutputTV>& vec, MonitorWall::tvMesResponse* response);	//�������
	void queryTVInfo(int TVChannel);			//��ѯ�������Ϣ

	//ǽ����
	void setSource(int Chn, int WinID, std::string ip, int port, std::string user, std::string pwd, int chnID);		//�����ź�Դ
	void delSource(int Chn, int WinID);		//��������ź�Դ
	void delSource();						//��������ź�Դ
	void delSource(int Chn);				//���һ���������е��ź�Դ
	int getWindowsInfo(int Chn);			//��ѯ������Ϣ
	void asyncSearchDev(MonitorWall::searchDevRequest* searchRequest);					//�첽��ѯ�豸
	int querySource(int Chn, MonitorWall::allWindowDisplayMesResponse* all_mes);			//��ѯָ�����ڵ��ź�Դ
	void querySource(int Chn, int WinID);													//��ѯָ�����ڵ���ʾԴ

	void setSplitStu(int Chn, int openNum);		//���÷���ʹ�õĽṹ��
	void closeWindow(int Chn, int windowID);	//����ǰ�ر����д���
	bool bindSource(int Chn, int WinID, std::string ip, std::string user, std::string pwd, int devChn, int port = 37777);	//�������ź�Դ
	void setSplitMode(int Chn, int openNum);		//����

	void getWindowStreamInfo(std::string blockID, int openWindowNum, std::shared_ptr<Block> pBlock);	//��������Ϣ
	void getStreamInfo();	//��������Ϣ

public:
	LLONG m_decoderLoginHandle = 0;;			//��������¼���

	//�����������Ϣ
	char m_decoderIp[64];
	WORD m_decoderPort;					// tcp ���Ӷ˿ڣ�����������¼�豸ҳ�� tcp �˿�����һ��
	char m_decoderName[64];
	char m_decoderPwd[64];

	//ǰ�˱����豸�������Ϣ
	char m_camIP[64];
	char m_camUser[64];
	char m_camPwd[64];
	WORD m_camPort;
	LLONG m_lRealHandle = 0;			//ʵʱԤ�����
	LLONG m_cameraLoginHandle;			//ǰ������ͷ��¼���
	int m_nChannelID;					//����ͷͨ����
	DH_RealPlayType m_emRealPlayType;	//ʵʱԤ������

	bool m_closeWin = false;

	int code = 0;						//c������
	std::vector<std::shared_ptr<Block>> m_vBlock;	//����Ϣ����
};

