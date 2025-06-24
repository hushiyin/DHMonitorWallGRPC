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

//窗口信息
class WindowInfo{
public:
	int m_windowID;			//窗口ID
	int m_bitRate;			//码率
	int m_frameRate;		//帧率
	int m_resolution;		//分辨率
	EM_NET_ENCODE_COMPRESSION m_compression;	//编码方式
	NET_VIDEOCHANNEL_STATE     m_state;     //窗口解码状态
};

//块信息
class Block{
public:
	int m_channelID;		//绑定的通道号
	int m_openWindowNum;	//开窗数量
	char szCompositeID[AV_CFG_Device_ID_Len];	// 单屏ID

	std::vector<std::shared_ptr<WindowInfo>> m_vwinInfo;	//窗口信息数组
};

class DHDecoder
{
public:
	DHDecoder(const char* ip = "", WORD port = 0, const char* user = "", const char* pwd = "");
	~DHDecoder();
	//初始化相关
	void initSDK();
	void getSDKVer();
	void uninitSDK();
	void longinDev();
	void logoutDev();
	int getLastErrorCode();
	int queryDecoderInfo();						//查询解码器信息

	//墙设置
	std::vector<wallConfig> getMonitorWallCfg();					//获取电视墙配置
	void setMonitorWallCfg(int blockNums, std::vector<BindOutputTV>& vec, MonitorWall::tvMesResponse* response);		//创建电视墙
	void setStruct(std::shared_ptr<AV_CFG_MonitorWall> pstuWall, int blockNums, std::vector<BindOutputTV>& vec, MonitorWall::tvMesResponse* response);	//绑定输出口
	void queryTVInfo(int TVChannel);			//查询输出口信息

	//墙操作
	void setSource(int Chn, int WinID, std::string ip, int port, std::string user, std::string pwd, int chnID);		//设置信号源
	void delSource(int Chn, int WinID);		//单个解绑信号源
	void delSource();						//批量解绑信号源
	void delSource(int Chn);				//解绑一个块上所有的信号源
	int getWindowsInfo(int Chn);			//查询窗口信息
	void asyncSearchDev(MonitorWall::searchDevRequest* searchRequest);					//异步查询设备
	int querySource(int Chn, MonitorWall::allWindowDisplayMesResponse* all_mes);			//查询指定窗口的信号源
	void querySource(int Chn, int WinID);													//查询指定窗口的显示源

	void setSplitStu(int Chn, int openNum);		//设置分屏使用的结构体
	void closeWindow(int Chn, int windowID);	//分屏前关闭所有窗口
	bool bindSource(int Chn, int WinID, std::string ip, std::string user, std::string pwd, int devChn, int port = 37777);	//分屏绑定信号源
	void setSplitMode(int Chn, int openNum);		//分屏

	void getWindowStreamInfo(std::string blockID, int openWindowNum, std::shared_ptr<Block> pBlock);	//窗口流信息
	void getStreamInfo();	//窗口流信息

public:
	LLONG m_decoderLoginHandle = 0;;			//解码器登录句柄

	//解码器相关信息
	char m_decoderIp[64];
	WORD m_decoderPort;					// tcp 连接端口，需与期望登录设备页面 tcp 端口配置一致
	char m_decoderName[64];
	char m_decoderPwd[64];

	//前端编码设备的相关信息
	char m_camIP[64];
	char m_camUser[64];
	char m_camPwd[64];
	WORD m_camPort;
	LLONG m_lRealHandle = 0;			//实时预览句柄
	LLONG m_cameraLoginHandle;			//前端摄像头登录句柄
	int m_nChannelID;					//摄像头通道号
	DH_RealPlayType m_emRealPlayType;	//实时预览类型

	bool m_closeWin = false;

	int code = 0;						//c错误码
	std::vector<std::shared_ptr<Block>> m_vBlock;	//块信息数组
};

