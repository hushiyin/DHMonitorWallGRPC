#pragma once
#include<iostream>
#include"dhnetsdk.h"

#include <grpcpp/grpcpp.h>
#include "MonitorWall.grpc.pb.h"

//设备断线回调哈数
void CALLBACK DisConnectFunc(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser);

//断线重连成功回调
// 通过CLIENT_SetAutoReconnect设置该回调函数，当已断线的设备重连成功时，SDK会调用该函数
void CALLBACK HaveReConnect(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser);

//实时预览回调函数
//通过CLIENT_SetRealDataCallbackEx设置该回调函数，当收到实时预览数据时，SDK会调用该函数
// 建议用户在此回调函数中只进行保存数据的操作，不建议用户在回调函数里直接对数据进行编解码等处理
// 即：将相应的数据拷贝到自己的存储空间，离开回调函数后再对数据做编解码等处理
void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE* pBuffer,
	DWORD dwBufSize, LONG param, LDWORD dwUser);

//异步回调函数
void CALLBACK MessDataCallBackFunc(LLONG lCommand, LPNET_CALLBACK_DATA lpData, LDWORD dwUser);

//异步搜索设备
void CALLBACK SearchCallback(DEVICE_NET_INFO_EX* pInfo, void* pUserData);