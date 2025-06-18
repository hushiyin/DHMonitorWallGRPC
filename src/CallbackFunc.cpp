#include"CallbackFunc.h"

void CALLBACK DisConnectFunc(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser){
	std::cout << "DisConnectFunc: lLoginID : " << lLoginID << std::endl;
}

void CALLBACK HaveReConnect(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser){
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

void CALLBACK SearchCallback(DEVICE_NET_INFO_EX* pInfo, void* pUserData){
	auto request = static_cast<MonitorWall::searchDevRequest*>(pUserData);
	if(pInfo){
		std::cout<<"find device: \n"
				 <<"Ip: "<<pInfo->szIP << "\n"
				 <<"nPort "<<pInfo->nPort << "\n" 
				 <<"szDeviceType : "<<pInfo->szDeviceType << "\n"
				 <<"szDetailType: "<<pInfo->szDetailType << "\n"
				 <<"szDevName: "<<pInfo->szDevName<< "\n" 
				 <<"nHttpPort: "<<pInfo->nHttpPort << "\n"
				 <<"wVideoInputCh "<<pInfo->wVideoInputCh << "\n"
				 <<std::endl;
		MonitorWall::devMes dev_mes;
		dev_mes.set_dev_ip(pInfo->szIP);
		dev_mes.set_dev_port(pInfo->nPort);
		dev_mes.set_dev_user("");
		dev_mes.set_dev_pwd("");
	}
}
