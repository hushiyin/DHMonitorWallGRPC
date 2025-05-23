#include"CallbackFunc.h"

void CALLBACK DisConnectFunc(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	std::cout << "DisConnectFunc: lLoginID : " << lLoginID << std::endl;
}

void CALLBACK HaveReConnect(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	std::cout << "HaveReConnect: lLoginID:" << lLoginID << std::endl;
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

