#pragma once
#include<iostream>
#include"dhnetsdk.h"

#include <grpcpp/grpcpp.h>
#include "MonitorWall.grpc.pb.h"

//�豸���߻ص�����
void CALLBACK DisConnectFunc(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser);

//���������ɹ��ص�
// ͨ��CLIENT_SetAutoReconnect���øûص����������Ѷ��ߵ��豸�����ɹ�ʱ��SDK����øú���
void CALLBACK HaveReConnect(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser);

//ʵʱԤ���ص�����
//ͨ��CLIENT_SetRealDataCallbackEx���øûص����������յ�ʵʱԤ������ʱ��SDK����øú���
// �����û��ڴ˻ص�������ֻ���б������ݵĲ������������û��ڻص�������ֱ�Ӷ����ݽ��б����ȴ���
// ��������Ӧ�����ݿ������Լ��Ĵ洢�ռ䣬�뿪�ص��������ٶ������������ȴ���
void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE* pBuffer,
	DWORD dwBufSize, LONG param, LDWORD dwUser);

//�첽�ص�����
void CALLBACK MessDataCallBackFunc(LLONG lCommand, LPNET_CALLBACK_DATA lpData, LDWORD dwUser);

//�첽�����豸
void CALLBACK SearchCallback(DEVICE_NET_INFO_EX* pInfo, void* pUserData);