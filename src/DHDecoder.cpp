#include "DHDecoder.h"
#include"dhnetsdk.h"
#include "dhconfigsdk.h"
#include "Utils.h"

DHDecoder::DHDecoder(const char* ip, WORD port, const char* user, const char* pwd)
{
	strcpy(m_decoderIp, ip);
	strcpy(m_decoderName, user);
	strcpy(m_decoderPwd, pwd);

	m_decoderPort = port;

	initSDK();
}

DHDecoder::~DHDecoder() {
	uninitSDK();
}

void DHDecoder::initSDK() {
	if (CLIENT_Init(DisConnectFunc, 0)) {
		std::cout << "SDK init success!" << std::endl;
	}
	else
	{
		std::cout << "SDK init failed!" << std::endl;
		return;
	}
	CLIENT_SetAutoReconnect(&HaveReConnect, 0);
}

void DHDecoder::getSDKVer() {
	std::cout << "SDK Version:" << CLIENT_GetSDKVersion() << std::endl;
}

void DHDecoder::logoutDev() {
	if (0 != m_decoderLoginHandle) {
		CLIENT_Logout(m_decoderLoginHandle);
		m_decoderLoginHandle = 0;
	}
}

void DHDecoder::uninitSDK() {
	std::cout << "uninit sdk!" << std::endl;
	logoutDev();
	CLIENT_Cleanup();
#ifndef REALPLAY
	stopPlayRealVideo();
#endif
}

#ifndef REALPLAY
void DHDecoder::startPlayRealVideo() {
	//获取控制台窗口句柄
	//HMODULE hKernel32 = GetModuleHandle(L"kernel32");
	//GetConsoleWindow = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32, "GetConsoleWindow");
	//m_hWnd = GetConsoleWindow();

	//开启实时预览
	m_nChannelID = 0;
	m_emRealPlayType = DH_RType_Realplay; // 实时预览

	//m_lRealHandle = CLIENT_RealPlayEx(m_lLoginHandle, 0, m_hWnd, m_emRealPlayType);	//SDK解码播放
	m_lRealHandle = CLIENT_RealPlayEx(m_cameraLoginHandle, 0, NULL, m_emRealPlayType);		//句柄传入NULL表示调用第三方接口播放
	if (m_lRealHandle == 0)
	{
		std::cout << "play real video error,error code:" << getLastErrorCode() << std::endl;
		return;
	}
	else
	{
		std::cout << "Real play handle:" << m_lRealHandle << std::endl;

		DWORD dwFlag = 0x00000001;	//原始数据标志
		if (FALSE == CLIENT_SetRealDataCallBackEx(m_lRealHandle, &RealDataCallBackEx, NULL, dwFlag)) {
			std::cout << "CLIENT_SetRealDataCallBackEx:failed:Error code:" << getLastErrorCode() << std::endl;
			return;
		}
		else
		{
			std::cout << "CLIENT_SetRealDataCallBackEx" << std::endl;
		}
	}
}
#endif
  
#ifndef REALPLAY
void DHDecoder::stopPlayRealVideo() {
	if (m_lRealHandle != 0) {
		if (CLIENT_StopRealPlayEx(m_lRealHandle)) {		//成功返回TRUE，失败返回FALSE
			std::cout << "stop play real video success!" << std::endl;
			m_lRealHandle = 0;
		}
		else
		{
			std::cout << "stop play real video failed!" << getLastErrorCode() << std::endl;
		}
	}
}
#endif

void DHDecoder::longinDev() {
	NET_DEVICEINFO_Ex stDevInfo = { 0 };

	// 登录设备
	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);

	strcpy(stInparam.szIP, m_decoderIp);
	strcpy(stInparam.szPassword, m_decoderPwd);
	strcpy(stInparam.szUserName, m_decoderName);

	stInparam.nPort = m_decoderPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);

	m_decoderLoginHandle = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);
	if (0 != m_decoderLoginHandle)
	{
		std::cout << "Dev login success! Login handle:" << m_decoderLoginHandle << std::endl;
	}
	else {
		std::cout << "Dev login failed! Error code:" << getLastErrorCode() << std::endl;
	}

	usleep(500);    //用户初次登录，需要进行一些初始化，建议等待一段时间
	std::cout << std::endl;
}

int DHDecoder::getLastErrorCode() {
	return CLIENT_GetLastError() & (0x7fffffff);	//这样输出_EX(x)中的x的值，这样处理得到的值更具有可读性
}

int DHDecoder::queryDecoderInfo() {
	if (m_decoderLoginHandle != 0) {
		//设置异步回到函数
		CLIENT_SetOperateCallBack(m_decoderLoginHandle, MessDataCallBackFunc, (LDWORD)0);

		DEV_DECODER_INFO stDevDecoderInfo = { 0 };
		BOOL bRet = CLIENT_QueryDecoderInfo(m_decoderLoginHandle, &stDevDecoderInfo, 2000);
		if (bRet) {
			std::cout << "Decoder type:         " << stDevDecoderInfo.szDecType << std::endl;
			std::cout << "TV nums:    	        " << stDevDecoderInfo.nMonitorNum << std::endl;
			std::cout << "Decoder channle nums: " << stDevDecoderInfo.nEncoderNum << std::endl;
		}
		return stDevDecoderInfo.nMonitorNum;
	}
}

void DHDecoder::queryDecoderChanel(int decoderID) {
	DEV_DECCHANNEL_STATE stChannelState = { 0 };
	BOOL bRet = CLIENT_QueryDecChannelFlux(m_decoderLoginHandle, decoderID, &stChannelState, 1000);//查询当前解码通道流信息
	if (!bRet)
	{
		std::cout << "CLIENT_QueryDecChannelFlux failed!, error code:" << getLastErrorCode() << std::endl;	//21：对返回的数据校验出错
	}
	else
	{
		//显示
		std::cout << std::endl;
		std::cout << "解码通道号：" << stChannelState.byDecoderID << std::endl;
		std::cout << "当前解码通道正在操作状态:" << stChannelState.byChnState << std::endl;	//0－空闲,1－实时预览,2－回放 3 - 轮巡
		std::cout << "当前数据帧率：" << stChannelState.byFrame << std::endl;
		std::cout << "解码通道数据总量" << stChannelState.nChannelFLux << std::endl;
		std::cout << "解码数据量" << stChannelState.nDecodeFlux << std::endl;
		std::cout << "当前数据分辨率" << stChannelState.szResolution << std::endl;
		std::cout << std::endl;
	}
}

void DHDecoder::switchEncoder(int decoderID) {
	//使解码器解码前端设备的数据，以指定的分割画面tv输出。
	DEV_ENCODER_INFO stuEncoderInfo = { 0 };

	strcpy(stuEncoderInfo.szDevIpEx, m_camIP);
	strcpy(stuEncoderInfo.szDevUser, m_camUser);
	strcpy(stuEncoderInfo.szDevPwd, m_camPwd);

	stuEncoderInfo.wDevPort = m_camPort;
	stuEncoderInfo.nDevChannel = 0;						//通道	
	stuEncoderInfo.nStreamType = 0;						//主码流
	stuEncoderInfo.bDevChnEnable = 1;					//解码通道使能
	stuEncoderInfo.byConnType = 0;						//-1: auto, 0：TCP；1：UDP；2：组播
	stuEncoderInfo.byWorkMode = 0;						//0：直连；1：转发

	//由于CLIENT_SwitchDecTVEncoder接口为异步方式，因此要自定义一个操作参数	pSwitchTvParam，以便在异步回调函数处理界面的变化。
	LLONG lOperateHandle = CLIENT_SwitchDecTVEncoder(m_decoderLoginHandle, decoderID, &stuEncoderInfo, 0/*pSwitchTvParam*/);
	if (lOperateHandle == 0)
	{
		std::cout << "Switch failed!, error code:" << getLastErrorCode() << std::endl;
	}
}

//////////////////////////////////////////墙设置////////////////////////////////////////////////////
void DHDecoder::getMonitorWallCfg(){
	const int nMaxJsonLen = MAX_BUF_LEN;
	std::shared_ptr<char[]> pszJsonBuf(new char[nMaxJsonLen]);
	
	if(CLIENT_GetNewDevConfig(m_decoderLoginHandle, CFG_CMD_MONITORWALL, -1, pszJsonBuf.get(), nMaxJsonLen, NULL, WAIT_TIME))
	{
		auto pstuWall = std::make_shared<AV_CFG_MonitorWall>();
		if( NULL == pstuWall )
		{
			return;
		}

		pstuWall->nStructSize = sizeof(AV_CFG_MonitorWall);

		for (int i = 0; i < AV_CFG_Max_Block_In_Wall; ++i)
		{
			pstuWall->stuBlocks[i].nStructSize = sizeof(AV_CFG_MonitorWallBlock);				
			pstuWall->stuBlocks[i].stuRect.nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
			for (int j = 0; j < AV_CFG_Max_TV_In_Block; ++j)
			{
				pstuWall->stuBlocks[i].stuTVs[j].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
			}
		}

		int nRetLen = 0;
		if(CLIENT_ParseData(CFG_CMD_MONITORWALL, pszJsonBuf.get(), pstuWall.get(), sizeof(AV_CFG_MonitorWall), &nRetLen))
		{
			SaveJsonToFile("monitor_wall_config.json", pszJsonBuf.get());

			// std::cout<<"pszJsonBuf"<<pszJsonBuf<<std::endl;

			std::cout<<std::endl;
			std::cout<<"#################### MonitorWall Config ####################"<<std::endl;
			std::cout<<"MonitorWall szName:              "<<pstuWall->szName<<std::endl;
			std::cout<<"MonitorWall nLine:               "<<pstuWall->nLine<<std::endl;
			std::cout<<"MonitorWall nColumn:             "<<pstuWall->nColumn<<std::endl;
			std::cout<<"MonitorWall nBlockCount:         "<<pstuWall->nBlockCount<<std::endl;
			std::cout<<"MonitorWall bDisable:            "<<pstuWall->bDisable<<std::endl;
			std::cout<<"MonitorWall szDesc:              "<<pstuWall->szDesc<<std::endl;
			std::cout<<"MonitorWall nCoordinateType:     "<<pstuWall->nCoordinateType<<std::endl;

			std::cout<<std::endl;
			std::cout<<"#################### Block Config ####################"<<std::endl;
			std::cout<<"MonitorWallBlock nLine:          "<<pstuWall->stuBlocks[0].nLine<<std::endl;
			std::cout<<"MonitorWallBlock nColumn:        "<<pstuWall->stuBlocks[0].nColumn<<std::endl;
			std::cout<<"MonitorWallBlock nTVCount:       "<<pstuWall->stuBlocks[0].nTVCount<<std::endl;	
			std::cout<<"MonitorWallBlock szName:         "<<pstuWall->stuBlocks[0].szName<<std::endl;

			std::cout<<std::endl;
			std::cout<<"#################### TVOut Config ####################"<<std::endl;
			std::cout<<"MonitorWallTVOut szDeviceID:     "<<pstuWall->stuBlocks[0].stuTVs[0].szDeviceID<<std::endl;
			std::cout<<"MonitorWallTVOut nChannelID:     "<<pstuWall->stuBlocks[0].stuTVs[0].nChannelID<<std::endl;
			std::cout<<"MonitorWallTVOut szName:         "<<pstuWall->stuBlocks[0].stuTVs[0].szName<<std::endl;
			std::cout<<std::endl;
		}
	}
}

void DHDecoder::setMonitorWallCfg(AV_int32 line, AV_int32 column, std::vector<BindOutputTV>& vec)
{
	int decoderOutTVNums = queryDecoderInfo();
	if(line*column > decoderOutTVNums){
		std::cout<<"line*column must <= " << decoderOutTVNums <<std::endl;
		return;
	}

	int blockNums = line*column;
	if(vec.size() != blockNums){
		std::cout<<"please bind all blocks!"<<std::endl;
		return;
	}

	const int nMaxJsonLen = MAX_BUF_LEN;
	std::shared_ptr<char[]> pszJsonBuf(new char[nMaxJsonLen]);
	//Wall
	auto pstuWall = std::make_shared<AV_CFG_MonitorWall>();
	setStruct(pstuWall, blockNums, line, column, vec);

	//电视墙配置
	if(CLIENT_PacketData(CFG_CMD_MONITORWALL, pstuWall.get(), sizeof(*pstuWall), pszJsonBuf.get(), nMaxJsonLen))
	{
		if(CLIENT_SetNewDevConfig(m_decoderLoginHandle, CFG_CMD_MONITORWALL, -1, pszJsonBuf.get(), strlen(pszJsonBuf.get()), NULL, NULL, WAIT_TIME))
		{
			std::cout<<"MonitorWall config success!"<<std::endl;
		}
		else{
			std::cout<<"MonitorWall config failed!"<<std::endl;
		}
	}
}

void DHDecoder::setStruct(std::shared_ptr<AV_CFG_MonitorWall> pstuWall, int blockNums, AV_int32 line, AV_int32 column, std::vector<BindOutputTV>& vec)
{
	pstuWall->nStructSize = sizeof(AV_CFG_MonitorWall);
	strcpy(pstuWall->szName, "monitorWall");
	pstuWall->nLine = 1;
	pstuWall->nColumn = 1;
	pstuWall->nBlockCount = blockNums;
	strcpy(pstuWall->szDesc, "MonitorWall");
	pstuWall->nCoordinateType = 2;

	//block
	int endCol = 0;		//标记行
	int endLine = 0;	//标记列
	for(int i = 0; i < blockNums; i++){
		AV_CFG_MonitorWallBlock& block = pstuWall->stuBlocks[i];
		block.nStructSize = sizeof(AV_CFG_MonitorWallBlock);
		block.nLine = 1;
		block.nColumn = 1;
		block.nTVCount = 1;
		sprintf(block.szName, "block%d", i+1);
		sprintf(block.szCompositeID, "Composite%d", i+1);

		block.stuRect.nStructSize = sizeof(AV_CFG_Rect);
		if(endCol < column)
		{
			//不换行，上/下边界的值不变（nTop nBottom）
			block.stuRect.nLeft = endCol;
			block.stuRect.nTop = endLine;
			block.stuRect.nRight = endCol;
			block.stuRect.nBottom = endLine;

			endCol++;
		}
		else{
			//换行
			endCol = 0;
			endLine++;

			block.stuRect.nLeft = endCol;
			block.stuRect.nTop = endLine;
			block.stuRect.nRight = endCol;
			block.stuRect.nBottom = endLine;

			endCol++;
		}

		//TVOut
		block.stuTVs[0].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
		//绑定输出口
		for(auto e : vec){
			if(e.BindLine == block.stuRect.nTop && e.BindCol == block.stuRect.nLeft)
			{
				block.stuTVs[0].nChannelID = e.BindCh;
			}
		}
		sprintf(block.stuTVs[0].szName, "screen%d", i+1);
	}
}

#if 0
template<class ...Args>
void DHDecoder::setMonitorWallCfg(AV_int32 line, AV_int32 column, Args ...args)
{
	int decoderOutTVNums = queryDecoderInfo();

	if(line*column > decoderOutTVNums)
	{
		std::cout<<"line*column must <= " << decoderOutTVNums <<std::endl;
		return;
	}

	int blockNums = line*column;
	if(sizeof...(args) != blockNums)	//获取参数包中参数的个数
	{
		std::cout<<"Args nums error!"<<std::endl;
		return;
	}

	BindOutputTV stuBind[] = { args... };	//列表初始化

	const int nMaxJsonLen = MAX_BUF_LEN;
	std::shared_ptr<char[]> pszJsonBuf(new char[nMaxJsonLen]);

	//Wall
	auto pstuWall = std::make_shared<AV_CFG_MonitorWall>();
	pstuWall->nStructSize = sizeof(AV_CFG_MonitorWall);
	strcpy(pstuWall->szName, "monitorWall");
	pstuWall->nLine = 1;
	pstuWall->nColumn = 1;
	pstuWall->nBlockCount = blockNums;
	strcpy(pstuWall->szDesc, "MonitorWall");
	pstuWall->nCoordinateType = 2;

	//block
	int endCol = 0;		//标记行
	int endLine = 0;	//标记列
	for(int i = 0; i < blockNums; i++){
		AV_CFG_MonitorWallBlock& block = pstuWall->stuBlocks[i];
		block.nStructSize = sizeof(AV_CFG_MonitorWallBlock);
		block.nLine = 1;
		block.nColumn = 1;
		block.nTVCount = 1;
		sprintf(block.szName, "block%d", i+1);
		sprintf(block.szCompositeID, "Composite%d", i+1);

		block.stuRect.nStructSize = sizeof(AV_CFG_Rect);
		if(endCol < column)
		{
			//不换行，上/下边界的值不变（nTop nBottom）
			block.stuRect.nLeft = endCol;
			block.stuRect.nTop = endLine;
			block.stuRect.nRight = endCol;
			block.stuRect.nBottom = endLine;

			endCol++;
		}
		else{
			//换行
			endCol = 0;
			endLine++;

			block.stuRect.nLeft = endCol;
			block.stuRect.nTop = endLine;
			block.stuRect.nRight = endCol;
			block.stuRect.nBottom = endLine;

			endCol++;
		}

		//TVOut
		block.stuTVs[0].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
		//绑定输出口
		for(auto e : stuBind){
			if(e.BindLine == block.stuRect.nTop && e.BindCol == block.stuRect.nLeft)
			{
				block.stuTVs[0].nChannelID = e.BindCh;
			}
		}
		sprintf(block.stuTVs[0].szName, "screen%d", i+1);
	}

	//电视墙配置
	if(CLIENT_PacketData(CFG_CMD_MONITORWALL, pstuWall.get(), sizeof(*pstuWall), pszJsonBuf.get(), nMaxJsonLen))
	{
		if(CLIENT_SetNewDevConfig(m_decoderLoginHandle, CFG_CMD_MONITORWALL, -1, pszJsonBuf.get(), strlen(pszJsonBuf.get()), NULL, NULL, WAIT_TIME))
		{
			std::cout<<"MonitorWall config success!"<<std::endl;
#if 0
			//融合屏配置
			std::shared_ptr<AV_CFG_SpliceScreen[]> pScreens(new AV_CFG_SpliceScreen[block.nTVCount]);

			for(int j = 0; j < block.nTVCount; j++)
			{
				pScreens[j].nStructSize = sizeof(AV_CFG_SpliceScreen);
				strcpy(pScreens[j].szWallName, pstuWall->szName);
				strcpy(pScreens[j].szName, pstuWall->stuBlocks[j].szName);
				pScreens[j].nBlockID = j;
			}

			if (CLIENT_PacketData(CFG_CMD_SPLICESCREEN, (char*)pScreens.get(), sizeof(AV_CFG_SpliceScreen)*block.nTVCount, pszJsonBuf.get(), nMaxJsonLen))
			{
				if (CLIENT_SetNewDevConfig(m_decoderLoginHandle, CFG_CMD_SPLICESCREEN, -1, pszJsonBuf.get(), strlen(pszJsonBuf.get()), NULL, NULL, 3000))
				{
					std::cout<<"Splice Screens config success!"<<std::endl;
				}
				
			}
#endif
		}
		else{
			std::cout<<"MonitorWall config failed!"<<std::endl;
		}
	}
}
#endif


//////////////////////////////////////////墙操作////////////////////////////////////////////////////
void DHDecoder::TVSwitch() {

	//控制TV1输出的画面分割，进行四画面分割，并把解码~4通道在该TV显示
	int nMonitorID = 0;
	int nSplitNum = 4;
	BYTE bEncoderChannel[4] = { 0 };
	bEncoderChannel[0] = 0; //解码通道
	bEncoderChannel[1] = 1;
	bEncoderChannel[2] = 2;
	bEncoderChannel[3] = 3;

	//控制解码器TV画面切割
	//由于CLIENT_CtrlDecTVScreen接口为异步方式，因此要自定义一个操作参数pCtrlTVParam，以便在异步回调函数处理界面的变化。
	int lOperateHandle = CLIENT_CtrlDecTVScreen(m_decoderLoginHandle, nMonitorID, TRUE, nSplitNum, bEncoderChannel, 4, 0/*pCtrlTVParam*/);
	if (lOperateHandle == 0)
	{
		std::cout << "CLIENT_CtrlDecTVScreen failed!, error code:" << getLastErrorCode() << std::endl;
	}
}

#ifdef IS_IMP
void DHDecoder::setVideoSource(){
	// CLIENT_SetSplitSource
}

void DHDecoder::queryVideoSource(){
	// CLIENT_GetSplitSource
}

void DHDecoder::querySplitMode(){
	// CLIENT_GetSplitMode
}

void DHDecoder::closeSplitWindow(){
	// CLIENT_CloseSplitWindow
}

void DHDecoder::setWindowposition(){
	// CLIENT_SetSplitWindowRect
}

void DHDecoder::openSplitWindow(){
	// CLIENT_OpenSplitWindow
}
#endif