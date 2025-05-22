#include"CallbackFunc.h"

void CALLBACK DisConnectFunc(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	std::cout << "DisConnectFunc: lLoginID : " << lLoginID << std::endl;
}

void CALLBACK HaveReConnect(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	std::cout << "HaveReConnect: lLoginID:" << lLoginID << std::endl;
}

void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE* pBuffer,
	DWORD dwBufSize, LONG param, LDWORD dwUser)
{
	// 若多个实时预览使用相同的数据回调函数，则用户可通过 lRealHandle 进行一一对应
	switch (dwDataType)
	{
	case 0:
		//原始音视频数据
		std::cout << std::endl;
		std::cout << "原始音视频混合数据" << std::endl;
		std::cout << "receive real data, param: :" << std::endl;
		std::cout << "lRealHandle:" << lRealHandle << std::endl;
		std::cout << "dwDataType:" << dwDataType << std::endl;
		std::cout << "pBuffer:" << pBuffer << std::endl;
		std::cout << "dwBufSize:" << dwBufSize << std::endl;
		std::cout << "param:" << param << std::endl;
		std::cout << "dwUser:" << dwUser << std::endl;
		std::cout << std::endl;
		break;
	case 1:
		//标准视频数据
		std::cout <<"标准视频数据" << std::endl;

		break;
	case 2:
		//yuv 数据
		std::cout << "yuv 数据" << std::endl;

			break;
	case 3:
		//pcm 音频数据
		std::cout << "pcm 音频数据" << std::endl;

		break;
	case 4:
		//原始音频数据
		std::cout << "原始音频数据" << std::endl;

		break;
	default:
		break;
	}
}

void CALLBACK MessDataCallBackFunc(LLONG lCommand, LPNET_CALLBACK_DATA lpData, LDWORD dwUser)
{
	if (lpData->userdata == NULL)
	{
		return;
	}
	switch (lCommand)
	{
	case RESPONSE_DECODER_CTRL_TV:// CLIENT_CtrlDecTVScreen接口
	{
		//……
	}
	break;
	case RESPONSE_DECODER_SWITCH_TV: // 对应CLIENT_SwitchDecTVEncoder接口
	{
		//……
	}
	break;
	case RESPONSE_DECODER_PLAYBACK: // 对应CLIENT_DecTVPlayback接口
	{
		//……
	}
	break;
	default:
		break;
	}
}

