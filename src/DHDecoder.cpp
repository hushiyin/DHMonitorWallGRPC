#include "DHDecoder.h"
#include"dhnetsdk.h"
#include "dhconfigsdk.h"
#include "Utils.h"

DHDecoder::DHDecoder(const char* ip, WORD port, const char* user, const char* pwd){
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
	if (!CLIENT_Init(DisConnectFunc, 0)) {
		code = getLastErrorCode();
		std::cout<<"sdk init failed"<<std::endl;
		return;
	}else{
		std::cout<<"init sdk successful!"<<std::endl;
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
}

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
	if (0 == m_decoderLoginHandle) {
		std::cout << "Dev login failed! Error code:" << getLastErrorCode() << std::endl;
		code = getLastErrorCode();
	}else{
		// std::cout<<"login successful! m_decoderLoginHandle: "<<m_decoderLoginHandle<<std::endl;
		usleep(500);    //用户初次登录，需要进行一些初始化，建议等待一段时间
		//设置异步回到函数
		CLIENT_SetOperateCallBack(m_decoderLoginHandle, MessDataCallBackFunc, (LDWORD)0);
	}
}

int DHDecoder::getLastErrorCode() {
	return CLIENT_GetLastError() & (0x7fffffff);	//这样输出_EX(x)中的x的值，这样处理得到的值更具有可读性
}

int DHDecoder::queryDecoderInfo() {
	DEV_DECODER_INFO stDevDecoderInfo = { 0 };
	if(!CLIENT_QueryDecoderInfo(m_decoderLoginHandle, &stDevDecoderInfo, 2000)){
		std::cout<<"query decoder info failed! error code: "<<getLastErrorCode()<<std::endl;
		code = getLastErrorCode();
		return -1;
	}
	return stDevDecoderInfo.nMonitorNum;
}

//////////////////////////////////////////墙设置////////////////////////////////////////////////////
std::vector<wallConfig> DHDecoder::getMonitorWallCfg() {
	std::vector<wallConfig> vec_wall_config;	//存储块信息的结构体数组
	const int nMaxJsonLen = MAX_BUF_LEN;
	std::shared_ptr<char[]> pszJsonBuf(new char[nMaxJsonLen]);
	
	if(CLIENT_GetNewDevConfig(m_decoderLoginHandle, CFG_CMD_MONITORWALL, -1, pszJsonBuf.get(), nMaxJsonLen, NULL, WAIT_TIME)){
		auto pstuWall = std::make_shared<AV_CFG_MonitorWall>();
		if(pstuWall ){
			pstuWall->nStructSize = sizeof(AV_CFG_MonitorWall);

			for (int i = 0; i < AV_CFG_Max_Block_In_Wall; ++i){
				pstuWall->stuBlocks[i].nStructSize = sizeof(AV_CFG_MonitorWallBlock);				
				pstuWall->stuBlocks[i].stuRect.nStructSize = sizeof(AV_CFG_MonitorWallTVOut);

				for (int j = 0; j < AV_CFG_Max_TV_In_Block; ++j){
					pstuWall->stuBlocks[i].stuTVs[j].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
				}
			}

			int nRetLen = 0;
			if(CLIENT_ParseData(CFG_CMD_MONITORWALL, pszJsonBuf.get(), pstuWall.get(), sizeof(AV_CFG_MonitorWall), &nRetLen)){
				// SaveJsonToFile("monitor_wall_config.json", pszJsonBuf.get());	//保存拆寻电视墙配置的json文件

				if(!m_vBlock.empty()){
					m_vBlock.clear();
					m_vBlock.shrink_to_fit();
				}

				for(int i = 0; i < pstuWall->nBlockCount; i++){
					//将每个块的信息都存起来
					wallConfig stuWall;
					stuWall.TVID = pstuWall->stuBlocks[i].stuTVs[0].nChannelID;
					stuWall.b_line = pstuWall->stuBlocks[i].stuRect.nTop;;
					stuWall.b_col = pstuWall->stuBlocks[i].stuRect.nLeft;
					vec_wall_config.emplace_back(stuWall);

					//用于查询窗口流信息
					auto pcBlock = std::make_shared<Block>();
					pcBlock->m_channelID = pstuWall->stuBlocks[i].stuTVs[0].nChannelID;		//Block类的通道号
					pcBlock->m_openWindowNum = getWindowsInfo(pcBlock->m_channelID);		//Block类的窗口数量
					strcpy(pcBlock->szCompositeID, pstuWall->stuBlocks[i].szCompositeID);	//Block类的单屏ID
					m_vBlock.emplace_back(pcBlock);
				}
			}else{
				code = getLastErrorCode();
			}
		}
	}else{
		code = getLastErrorCode();
	}
	return vec_wall_config;
}

void DHDecoder::setMonitorWallCfg(int blockNums, std::vector<BindOutputTV>& vec, MonitorWall::tvMesResponse* response) {
	int decoderOutTVNums = queryDecoderInfo();
	if(blockNums > decoderOutTVNums){
		std::cout<<"blockNums must <= " << decoderOutTVNums <<std::endl;
		return;
	}

	if(vec.size() != blockNums){
		std::cout<<"please bind all blocks!"<<std::endl;
		return;
	}

	for(int i =0; i < decoderOutTVNums; i++){		//初始化所有的输出口的输出口,初始化状态下都是未绑定，并且对应的块坐标都为（-1，-1）
		MonitorWall::tvList tv_list;
		tv_list.set_out_tv_id(i);
		tv_list.set_is_bind(false);
		tv_list.set_block_line(-1);
		tv_list.set_block_col(-1);
		*response->add_tv_list() = tv_list;
	}

	const int nMaxJsonLen = MAX_BUF_LEN;
	std::shared_ptr<char[]> pszJsonBuf(new char[nMaxJsonLen]);
	//Wall
	auto pstuWall = std::make_shared<AV_CFG_MonitorWall>();
	setStruct(pstuWall, blockNums, vec, response);

	//电视墙配置
	if(CLIENT_PacketData(CFG_CMD_MONITORWALL, pstuWall.get(), sizeof(*pstuWall), pszJsonBuf.get(), nMaxJsonLen)){
		if(CLIENT_SetNewDevConfig(m_decoderLoginHandle, CFG_CMD_MONITORWALL, -1, pszJsonBuf.get(), strlen(pszJsonBuf.get()), NULL, NULL, WAIT_TIME)){
			code = getLastErrorCode();
		}
	}
}

void DHDecoder::setStruct(std::shared_ptr<AV_CFG_MonitorWall> pstuWall, int blockNums, std::vector<BindOutputTV>& vec, MonitorWall::tvMesResponse* response) {
	if(blockNums != vec.size()){	
		std::cout<<"block nums error"<<std::endl;
		return;
	}

	pstuWall->nStructSize = sizeof(AV_CFG_MonitorWall);
	strcpy(pstuWall->szName, "monitorWall");
	pstuWall->nLine = 1;
	pstuWall->nColumn = 1;
	pstuWall->nBlockCount = blockNums;
	strcpy(pstuWall->szDesc, "MonitorWall");
	pstuWall->nCoordinateType = 2;

	//block
	for(int i = 0; i < blockNums; i++){
		AV_CFG_MonitorWallBlock& block = pstuWall->stuBlocks[i];
		block.nStructSize = sizeof(AV_CFG_MonitorWallBlock);
		block.nLine = 1;
		block.nColumn = 1;
		block.nTVCount = 1;
		sprintf(block.szName, "block%d", i+1);
		sprintf(block.szCompositeID, "Composite%d", i+1);

		block.stuRect.nStructSize = sizeof(AV_CFG_Rect);
		block.stuTVs[0].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);		//一个输出口只能绑定一个TV

		block.stuRect.nLeft = vec[i].BindCol;
		block.stuRect.nTop = vec[i].BindLine;
		block.stuRect.nRight = vec[i].BindCol;
		block.stuRect.nBottom = vec[i].BindLine;

		for(auto& inputBindStruct : vec){	//遍历传入的需要绑定的结构体信息
			if(inputBindStruct.BindLine == block.stuRect.nTop && inputBindStruct.BindCol == block.stuRect.nLeft){	//判断当前的块是否是需要绑定的块
				block.stuTVs[0].nChannelID = inputBindStruct.BindCh;	//绑定输出口

				//遍历所有输出口
				for(auto& all_tv : *response->mutable_tv_list()){		
					if(all_tv.out_tv_id() == inputBindStruct.BindCh){
						all_tv.set_is_bind(true);			//更新输出口状态
						all_tv.set_block_col(inputBindStruct.BindCol);
						all_tv.set_block_line(inputBindStruct.BindLine);
					}
				}
			}
		}
		sprintf(block.stuTVs[0].szName, "screen%d", i+1);
	}
}

void DHDecoder::queryTVInfo(int TVChannel) {
	DEV_DECODER_TV tv = { 0 };
	if(!CLIENT_QueryDecoderTVInfo(m_decoderLoginHandle, TVChannel, &tv, WAIT_TIME)){
		std::cout<<"query TV info failed! error code: "<<getLastErrorCode()<<std::endl;
		code = getLastErrorCode();
		return;
	}
	else{
		std::cout<<"tv id: "<<tv.nID<<"\n"
				 <<"split num: "<<tv.nSplitType<<std::endl;
		std::cout<<std::endl;
		for(int i = 0; i < sizeof(tv.stuDevInfo)/sizeof(tv.stuDevInfo[0]); i++)
		{
			std::cout<<"dev ip: : "<<tv.stuDevInfo[i].szDevIp <<"\n"
					 <<"camera channel: "<<tv.stuDevInfo[i].nDevChannel<<"\n"
					 <<"split num: "<<tv.stuDevInfo[i].nStreamType<<"\n"
					 <<"stream type: "<<tv.stuDevInfo[i].nStreamType<<std::endl;
			std::cout<<std::endl;
		}
	}
}

//////////////////////////////////////////墙操作////////////////////////////////////////////////////

//绑定信号源
void DHDecoder::setSource(int Chn, int WinID, std::string ip, int port, std::string user, std::string pwd, int chnID) {

	if(getWindowsInfo(Chn) > 0){
		bindSource(Chn, WinID, ip, user, pwd, chnID, port);			//信号源上墙
	}else{
		//开窗
		auto pstuInParam = std::make_shared<DH_IN_SPLIT_OPEN_WINDOW>();
		pstuInParam->dwSize = sizeof(DH_IN_SPLIT_OPEN_WINDOW);
		pstuInParam->nChannel = Chn;
		pstuInParam->stuRect.left = 0;
		pstuInParam->stuRect.top = 0;
		pstuInParam->stuRect.right = 8192;
		pstuInParam->stuRect.bottom = 8192;

		auto pstuOutParam = std::make_shared<DH_OUT_SPLIT_OPEN_WINDOW>();
		pstuOutParam->dwSize = sizeof(DH_OUT_SPLIT_OPEN_WINDOW);

		if (!CLIENT_OpenSplitWindow(m_decoderLoginHandle, pstuInParam.get(), pstuOutParam.get(), WAIT_TIME)){
			std::cout<<"open split windows failed! error code: "<<getLastErrorCode()<<std::endl;
			code = getLastErrorCode();
			return;
		}else{
			bindSource(Chn, WinID, ip, user, pwd, chnID, port);			//信号源上墙
			// auto pstuSrc = std::make_shared<DH_SPLIT_SOURCE>();
			// pstuSrc->dwSize = sizeof(DH_SPLIT_SOURCE);
			// pstuSrc->bEnable = TRUE;
			// strcpy(pstuSrc->szIp, ip.c_str());
			// pstuSrc->nPort = port;
			// strcpy(pstuSrc->szUserEx, user.c_str());
			// strcpy(pstuSrc->szPwdEx, pwd.c_str());
			// pstuSrc->nChannelID = chnID;

			// if(!CLIENT_SetSplitSource(m_decoderLoginHandle, Chn, WinID, pstuSrc.get(), 1, WAIT_TIME)){
			// 	std::cout<<"set source failed!"<<getLastErrorCode()<<std::endl;
			// 	code = getLastErrorCode();
			// }
		}
	}
}

//单个解绑信号源
void DHDecoder::delSource(int Chn, int WinID) {
	//解绑信号源
	DH_SPLIT_SOURCE stuSrc = { sizeof(DH_SPLIT_SOURCE) };
	if(!CLIENT_SetSplitSource(m_decoderLoginHandle, Chn, WinID, &stuSrc, 1, WAIT_TIME)){
		std::cout<<"del source failed!error code: "<<getLastErrorCode()<<std::endl;
		code = getLastErrorCode();
		return;
	}

	closeWindow(Chn, WinID);
}

//解绑一个块的所有信号源
void DHDecoder::delSource(int Chn){
	if(getWindowsInfo(Chn) > 0 && getWindowsInfo(Chn) == vwinID.size())	//有打开的窗口，并且开窗的数量等于保存的窗口ID数
	{
		for(int j = 0; j < vwinID.size(); j++){
			delSource(Chn, vwinID[j]);
		}
	}
}

//批量解绑信号源
void DHDecoder::delSource() {
	int TVNums = queryDecoderInfo();
	for(int i = 0; i < TVNums; i++){
		if(getWindowsInfo(i) > 0 && getWindowsInfo(i) == vwinID.size()){	//有打开的窗口，并且开窗的数量等于保存的窗口ID数
			for(int j = 0; j < vwinID.size(); j++){
				delSource(i, vwinID[j]);
			}
		}
	}
}

//获取窗口数和窗口ID
int DHDecoder::getWindowsInfo(int Chn) {
	int winNums = -1;
	//输入参数
	auto pstuInParam = std::make_shared<DH_IN_SPLIT_GET_WINDOWS>();
	pstuInParam->dwSize = sizeof(DH_IN_SPLIT_GET_WINDOWS);
	pstuInParam->nChannel = Chn;
	
	//输出参数
	auto pstuOutParam = std::make_shared<DH_OUT_SPLIT_GET_WINDOWS>();
	pstuOutParam->dwSize = sizeof(DH_OUT_SPLIT_GET_WINDOWS);
	DH_WINDOW_COLLECTION winInfo[DH_MAX_SPLIT_WINDOW];
	for(int j = 0; j < DH_MAX_SPLIT_WINDOW; j++){
		winInfo[j].dwSize = sizeof(DH_WINDOW_COLLECTION);
	}
	memcpy(pstuOutParam->stuWindows.stuWnds, winInfo, sizeof(winInfo));

	if(!CLIENT_GetSplitWindowsInfo(m_decoderLoginHandle, pstuInParam.get(), pstuOutParam.get(), WAIT_TIME)){
		std::cout<<"get windows info failed! error code: "<<getLastErrorCode()<<std::endl;
		code = getLastErrorCode();
		return -1;
	}else{
		winNums = pstuOutParam->stuWindows.nWndsCount;		//窗口数量

		if(!vwinID.empty()){
			vwinID.clear();
			vwinID.shrink_to_fit();	//释放未使用的内存
		}
		for(int i = 0; i < winNums; i++){
			int winID = pstuOutParam->stuWindows.stuWnds[i].nWindowID;
			vwinID.emplace_back(winID);
		}
	}
	return winNums;
}


//异步搜素设备
void DHDecoder::asyncSearchDev(MonitorWall::searchDevRequest* searchRequest) {
	CLIENT_StartSearchDevices(
        SearchCallback, 
        static_cast<void*>(searchRequest),       // 用户数据（可传递自定义对象）
        nullptr       		 // 不指定本地IP（自动选择网卡）
    );
}

//查询指定通道所有窗口的信号源信息
int DHDecoder::querySource(int Chn, MonitorWall::allWindowDisplayMesResponse* all_mes) {
	std::vector<wallConfig> vec_wall = getMonitorWallCfg();	//查询窗口信息,主要是已绑定的信号源

	MonitorWall::oneWindeosDispalyMes one_win_mes;
	int nRealWindow = 0;
	int nMaxSplitWindow = 64;
	std::shared_ptr<DH_SPLIT_SOURCE[]> pstuSrc(new DH_SPLIT_SOURCE[nMaxSplitWindow]);

	for (int j = 0; j < nMaxSplitWindow; ++j){
		pstuSrc[j].dwSize = sizeof(DH_SPLIT_SOURCE);
	}

	if(!CLIENT_GetSplitSource(m_decoderLoginHandle, Chn, -1, pstuSrc.get(), nMaxSplitWindow, &nRealWindow, WAIT_TIME)){
		code = getLastErrorCode();
		return -1;
	}else{
		one_win_mes.set_out_tv_id(Chn);
		one_win_mes.set_win_num(nRealWindow);
		one_win_mes.set_is_bind(true);

		for(auto& query_mes: vec_wall){
			if(query_mes.TVID == Chn){
				one_win_mes.set_block_col(query_mes.b_col);
				one_win_mes.set_block_line(query_mes.b_line);
			}
		}

		if(nRealWindow > 0){		//有开窗,就保存摄像头信息
			for(int i = 0; i < nRealWindow; i++){
				if(pstuSrc[i].szIp[0] != '\0'){
					MonitorWall::cameraMessage cam_mes;
					cam_mes.set_camera_name(pstuSrc[i].szDevName);
					cam_mes.set_stream_type(pstuSrc[i].nStreamType);
					cam_mes.set_camera_ip(pstuSrc[i].szIp);
					cam_mes.set_camera_channel(pstuSrc[i].nChannelID);
					cam_mes.set_win_id(i);

					*one_win_mes.add_cam_mes() = cam_mes;		//摄像头信息添加进数组
				}
			}
			all_mes->add_one_win_mes()->CopyFrom(one_win_mes);		//单个窗口的显示信息添加进数组
		}
	}
	return nRealWindow;
}

//查询指定窗口的信号源信息
void DHDecoder::querySource(int Chn, int WinID){
	int nRealWindow = 0;
	int nMaxSplitWindow = 64;
	std::shared_ptr<DH_SPLIT_SOURCE[]> pstuSrc(new DH_SPLIT_SOURCE[nMaxSplitWindow]);

	for (int j = 0; j < nMaxSplitWindow; ++j)
	{
		pstuSrc[j].dwSize = sizeof(DH_SPLIT_SOURCE);
	}

	if(!CLIENT_GetSplitSource(m_decoderLoginHandle, Chn, WinID, pstuSrc.get(), nMaxSplitWindow, &nRealWindow, WAIT_TIME)){
		code = getLastErrorCode();
		return;
	}else{
		for(int i = 0; i < nRealWindow; i++)
		{
			//保存已经绑定的信号源
			bindSourceInfo stuBindSource;
			if(pstuSrc[i].szIp[0] != '\0'){
				strcpy(stuBindSource.sourceIP, pstuSrc[i].szIp);
				// stuBindSource.sourcePort = pstuSrc[i].nPort;
				strcpy(stuBindSource.sourceName, pstuSrc[i].szUser);
				strcpy(stuBindSource.sourcePwd, pstuSrc[i].szUser);
				stuBindSource.sourceChaneel = pstuSrc[i].nChannelID;
				vecStuBindSource.emplace_back(stuBindSource);
			}
		}
	}
}

////////////////////////////////////////////////////分屏//////////////////////////////////////////////////////
void DHDecoder::setSplitStu(int Chn, int openNum){
	switch (openNum){
	case 1:
		openWindowSqrt(Chn, openNum);
		break;

	case 2:
		openWindowsTwo(Chn);
		break;

	case 4:
		openWindowSqrt(Chn, openNum);
		break;

	case 6:
		openWindowspecial(Chn, openNum);
		break;

	case 8:
		openWindowspecial(Chn, openNum);
		break;

	case 9:
		openWindowSqrt(Chn, openNum);
		break;

	case 16:
		openWindowSqrt(Chn, openNum);
		break;

	case 25:
		openWindowSqrt(Chn, openNum);
		break;
	
	default:
		code = 410;
		break;
	}
}

//关闭开窗
void DHDecoder::closeWindow(int Chn, int windowID){
	auto pstuInParam = std::make_shared<DH_IN_SPLIT_CLOSE_WINDOW>();
	pstuInParam->dwSize = sizeof(DH_IN_SPLIT_CLOSE_WINDOW);
	pstuInParam->nChannel = Chn;
	pstuInParam->nWindowID = windowID;
	
	auto pstuOutParam = std::make_shared<DH_OUT_SPLIT_CLOSE_WINDOW>();
	pstuOutParam->dwSize = sizeof(DH_OUT_SPLIT_CLOSE_WINDOW);
	if(!CLIENT_CloseSplitWindow(m_decoderLoginHandle, pstuInParam.get(), pstuOutParam.get(), WAIT_TIME)){
		std::cout<<"close split windows failed! error code: "<<getLastErrorCode()<<std::endl;
		code = getLastErrorCode();
		return;
	}
}

//绑定信号源   用于分屏操作
bool DHDecoder::bindSource(int Chn, int WinID, std::string ip, std::string user, std::string pwd, int devChn, int port){
	auto pstuSrc = std::make_shared<DH_SPLIT_SOURCE>();
	pstuSrc->dwSize = sizeof(DH_SPLIT_SOURCE);
	pstuSrc->bEnable = TRUE;
	strcpy(pstuSrc->szIp, ip.c_str());
	pstuSrc->nPort = port;
	// pstuSrc->nPort = 37777;
	strcpy(pstuSrc->szUserEx, user.c_str());
	strcpy(pstuSrc->szPwdEx, pwd.c_str());
	// pstuSrc->nStreamType = streamType;
	pstuSrc->nChannelID = devChn;

	if(!CLIENT_SetSplitSource(m_decoderLoginHandle, Chn, WinID, pstuSrc.get(), 1, WAIT_TIME)){
		// std::cout<<"set source failed!"<<getLastErrorCode()<<std::endl;
		code = getLastErrorCode();
		return false;
	}
	return true;
}

void DHDecoder::setSplitMode(int Chn, int openNum){
	//处理已绑定信号源的情况
	getWindowsInfo(Chn);	//查询窗口数量,设置vwinID

	if(!vecStuBindSource.empty()){
		vecStuBindSource.clear();
		vecStuBindSource.shrink_to_fit();
	}

	if(!vwinID.empty()){
		for(int j = 0; j < vwinID.size(); j++){		//遍历窗口号
			querySource(Chn, j);		//保存已经绑定的信号源，设置vecStuBindSource

			closeWindow(Chn, vwinID[j]);	//关闭窗口
		}
	}

	setSplitStu(Chn, openNum);		//设置开窗数量

	//开窗
	if(!vecOpenInputPara.empty() && !vecOpenOutputPara.empty() && vecOpenInputPara.size() == vecOpenOutputPara.size()){
		for(int i = 0; i < vecOpenInputPara.size(); i++){
			if(!CLIENT_OpenSplitWindow(m_decoderLoginHandle, &vecOpenInputPara[i], &vecOpenOutputPara[i], WAIT_TIME)){
				code = getLastErrorCode();
				return;
			}
		}
	}

	int minNUm = (vecStuBindSource.size() < getWindowsInfo(Chn)) ? vecStuBindSource.size() : getWindowsInfo(Chn);	//处理信号源大于新开窗的情况
	//恢复信号源
	if(!vecStuBindSource.empty()){
		for(int k = 0; k < minNUm; k++){
			bindSource(Chn, k, vecStuBindSource[k].sourceIP, vecStuBindSource[k].sourceName, 
						vecStuBindSource[k].sourcePwd , vecStuBindSource[k].sourceChaneel/*, vecStuBindSource[k].sourcePort*/);
		}
	}
}

void DHDecoder::getWindowStreamInfo(std::string blockID, int openWindowNum, std::shared_ptr<Block> pBlock) {
    // 入参初始化
    auto pInput = std::make_shared<NET_IN_MW_GET_WINODW_INFO>();
    pInput->dwSize = sizeof(NET_IN_MW_GET_WINODW_INFO);
	pInput->pszCompositeID = blockID.c_str();

    // 出参初始化
    auto pOutput = std::make_shared<NET_OUT_MW_GET_WINDOW_INFO>();
    pOutput->dwSize = sizeof(NET_OUT_MW_GET_WINDOW_INFO);
    pOutput->nVideoInfoNum = openWindowNum;
    pOutput->pNetVideoChannelInfo = new NET_MW_GET_WINDOW_INFO[pOutput->nVideoInfoNum];
    
    BOOL bRet = CLIENT_MonitorWallGetWindowInfo(
        m_decoderLoginHandle, 
        pInput.get(), 
        pOutput.get(), 
        WAIT_TIME
    );

    if (!bRet) {
		code = getLastErrorCode();
        std::cerr << "CLIENT_MonitorWallGetWindowInfo failed! Error code: " 
                  << getLastErrorCode() << std::endl;
    } else {
		if(pOutput->nRetVideoInfoNum > 0){
			for (int i = 0; i < pOutput->nRetVideoInfoNum; ++i) {
				auto pwinStreamInfo = std::make_shared<WindowInfo>();
				pwinStreamInfo->m_windowID = i;
				pwinStreamInfo->m_bitRate = pOutput->pNetVideoChannelInfo[i].unBitrate;
				pwinStreamInfo->m_frameRate = pOutput->pNetVideoChannelInfo[i].nFrame;
				pwinStreamInfo->m_resolution = pOutput->pNetVideoChannelInfo[i].emResolution;
				pwinStreamInfo->m_compression = pOutput->pNetVideoChannelInfo[i].emCompression;		//编码方式
				pwinStreamInfo->m_state = pOutput->pNetVideoChannelInfo[i].emState;					//窗口解码状态

				pBlock->m_vwinInfo.emplace_back(pwinStreamInfo);
			}
		}
        
    }

    delete[] pOutput->pNetVideoChannelInfo;
	return;
}

void DHDecoder::getStreamInfo(){
	getMonitorWallCfg();

	if(!m_vBlock.empty()){
		for(auto blockMes : m_vBlock){
			if(blockMes->m_openWindowNum > 0){
				getWindowStreamInfo(blockMes->szCompositeID, blockMes->m_openWindowNum, blockMes);
			}
		}
	}
}

