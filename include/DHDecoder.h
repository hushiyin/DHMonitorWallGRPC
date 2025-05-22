#pragma once
#include <iostream>
#include <string.h>
#include <unistd.h> 
#include <memory>
#include <vector>
#include "CallbackFunc.h"
#include "dhconfigsdk.h"
#include "Utils.h"

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
	void decoderData();								//解码数据
	void queryDecoderChanel(int decoderID);			//查询当前解码通道流信息
	void switchEncoder(int decoderID);				//切换解码器连接的编码器的信息

	//墙设置
	void getMonitorWallCfg();						//获取电视墙配置
	void setMonitorWallCfg(AV_int32 line, AV_int32 column, std::vector<BindOutputTV>& vec);		//创建电视墙
	void setStruct(std::shared_ptr<AV_CFG_MonitorWall> pstuWall, int blockNums, AV_int32 line, AV_int32 column, std::vector<BindOutputTV>& vec);	//绑定输出口
	void TVSwitch();

	//墙操作

#ifdef IS_IMPL
	//墙擦操作
	void setVideoSource();							//添加信号源
	void queryVideoSource();						//查询视频源

	void querySplitMode();							//查询分割模式
	void closeSplitWindow();						//关闭自由分割模式下打开的窗口
	void setWindowposition();						//设置窗口位置
	void openSplitWindow();							//自由分割模式下打开新窗口
#endif


	//实时预览相关（取前端摄像头的码流，暂时不用）
	//typedef HWND(WINAPI* PROCGETCONSOLEWINDOW)();
	//PROCGETCONSOLEWINDOW GetConsoleWindow;
	void startPlayRealVideo();			//开启实时预览
	void stopPlayRealVideo();			//停止实时预览

public:
	LLONG m_decoderLoginHandle;			//解码器登录句柄

public:
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
};

