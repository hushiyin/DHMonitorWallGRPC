#include <fstream>
#include "Utils.h"
#include <vector>
#include <string>

// using grpc::Status;

std::vector<decoderInfo> vecDecoderInfo;
std::vector<int> vwinID;
MonitorWall::allWindowDisplayMesResponse* all_mes;
std::vector<DH_IN_SPLIT_OPEN_WINDOW> vecOpenInputPara;
std::vector<DH_OUT_SPLIT_OPEN_WINDOW> vecOpenOutputPara;
std::vector<bindSourceInfo> vecStuBindSource;

std::map<int, std::string> codeMap = {
    {-1, "未知错误"},
    {0, "没有错误"},
    {5, "打开通道失败"},
    {6, "关闭通道失败"},
    {7, "用户参数不合法"},
    {8, "SDK初始化出错"},
    {21, "对返回数据的校验出错"},
    {25, "无操作权限"},
    {29, "网络SDK未经初始化"},
    {31, "查询结果为空"},
    {86, "输入密码错误超过限制次数"},
    {100, "密码不正确"},
    {101, "帐户不存在"},
    {102, "等待登录返回超时"},
    {103, "帐号已登录"},
    {107, "连接主机失败"},
    {410, "设备不支持的分割模式"},
    {516, "数据发送失败"}
};

//编码方式
std::map<int, std::string> encoderCompression = {
    {0, "未知"},
    {1, "H264"},
    {2, "MPEG4"},
    {3, "MJPEG"},
    {4, "SVAC"},
    {5, "HIK"},
    {6, "H265"}
};

//分辨率
std::map<int, std::string> resolution = {
    {0, "704*576(PAL)  704*480(NTSC)"},
    {1, "352*576(PAL)  352*480(NTSC)"},
    {2, "704*288(PAL)  704*240(NTSC)"},
    {3, "352*288(PAL)  352*240(NTSC)"},
    {4, "176*144(PAL)  176*120(NTSC)"},
    {5, "640*480"},
    {6, "320*240"},
    {7, "480*480"},
    {8, "160*128"},
    {9, "800*592"},
    {10, "1024*768"},
    {11, "1280*800"},
    {12, "1280*1024"},
    {13, "1600*1024 "},
    {14, "1600*1200"},
    {15, "1920*1200"},
    {16, "240*192,ND1"},
    {17, "1280*720"},
    {18, "1920*1080"},
    {19, "1280*960"},
    {20, "1872*1408,2_5M"},
    {21, "3744*1408"},
    {22, "2048*1536"},
    {23, "2432*2050"},
    {24, "1216*1024"},
    {25, "1408*1024"},
    {26, "3296*2472"},
    {27, "2560*1920(5_1M)"},
    {28, "960*576(PAL) 960*480(NTSC)"},
    {29, "960*720"},
    {30, "640*360"},
    {31, "320*180"},
    {32, "160*90"},
    {33, "960*540"},
    {34, "640*352"},
    {35, "640*400"},
    {36, "320*192 "},
    {37, "320*176"},
    {38, "800*600"},
    {39, "2560*1440"},
    {40, "2304*1296"},
    {41, "2592*1520"},
    {42, "4000*3000"},
    {43, "2880*2880"},
    {44, "2880*2160"},
    {45, "2688*1520"},
    {46, "2592*1944"},
    {47, "3072*1728"},
    {48, "3072*2048"},
    {49, "3840*2160"},
    {50, "160*120"}
};

int globalCode = 0;;

void SaveJsonToFile(const std::string& filename, const char* jsonData)
{
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << jsonData;
        outFile.close();
        std::cout << "JSON data save to file " << filename << std::endl;
    } else {
        std::cerr << "cannot open " << filename << std::endl;
    }
}

void saveLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd)
{
    decoderInfo gstuSaveInfo(ip.c_str(), port, user.c_str(), pwd.c_str());
    vecDecoderInfo.emplace_back(gstuSaveInfo);
}

bool queryLoginInfo(const std::string ip, int port, const std::string user, const std::string pwd)
{
    decoderInfo gstuQueryInfo(ip.c_str(), port, user.c_str(), pwd.c_str());

    if(!vecDecoderInfo.empty()){
        for(int i = 0; i < vecDecoderInfo.size(); i++){
            if(vecDecoderInfo[i] == gstuQueryInfo)
            {
                return true;    //登录成功
            }
        }
        return false;   //未登录
    }else{
        return false;   //未登录
    }
}

//清空vector
void clearVector(std::vector<DH_IN_SPLIT_OPEN_WINDOW>& vecInput, std::vector<DH_OUT_SPLIT_OPEN_WINDOW>& vecOutput){
    if(!vecInput.empty()){
        vecInput.clear();
        vecInput.shrink_to_fit();
    }
    if(!vecOutput.empty()){
        vecOutput.clear();
        vecOutput.shrink_to_fit();
    }
}

// 2分屏
void openWindowsTwo(int Chn){
    clearVector(vecOpenInputPara, vecOpenOutputPara);

    int rect = SWKJ_BLOCK_SIZE / 2;

    for(int i = 0; i < 2; i++){
        DH_IN_SPLIT_OPEN_WINDOW stuOneInputWindow;
        stuOneInputWindow.dwSize = sizeof(DH_IN_SPLIT_OPEN_WINDOW);
        stuOneInputWindow.nChannel = Chn;
        stuOneInputWindow.stuRect.left = rect * i;
        stuOneInputWindow.stuRect.top = 0;
        stuOneInputWindow.stuRect.right = rect * (i + 1);
        stuOneInputWindow.stuRect.bottom = SWKJ_BLOCK_SIZE;
        vecOpenInputPara.emplace_back(stuOneInputWindow);

        DH_OUT_SPLIT_OPEN_WINDOW stuOneOutputWindow;
        stuOneOutputWindow.dwSize = sizeof(DH_OUT_SPLIT_OPEN_WINDOW);
        vecOpenOutputPara.emplace_back(stuOneOutputWindow);
    }
}

// 根号分屏
void openWindowSqrt(int Chn, int openNum){
    clearVector(vecOpenInputPara, vecOpenOutputPara);
    int rect = SWKJ_BLOCK_SIZE / sqrt(openNum);

    for(int i = 0; i < sqrt(openNum); i++){     //遍历行
        for(int j = 0; j < sqrt(openNum); j++){
            DH_IN_SPLIT_OPEN_WINDOW stuOneInputWindow;
            stuOneInputWindow.dwSize = sizeof(DH_IN_SPLIT_OPEN_WINDOW);
            stuOneInputWindow.nChannel = Chn;
            stuOneInputWindow.stuRect.left = rect * j;
            stuOneInputWindow.stuRect.top = rect * i;
            stuOneInputWindow.stuRect.right = rect * (j + 1);
            stuOneInputWindow.stuRect.bottom = rect * (i + 1);
            vecOpenInputPara.emplace_back(stuOneInputWindow);

            DH_OUT_SPLIT_OPEN_WINDOW stuOneOutputWindow;
            stuOneOutputWindow.dwSize = sizeof(DH_OUT_SPLIT_OPEN_WINDOW);
            vecOpenOutputPara.emplace_back(stuOneOutputWindow);
        }
    }
}

//特殊分屏（6分屏和8分屏）
void openWindowspecial(int Chn, int openNum){
    clearVector(vecOpenInputPara, vecOpenOutputPara);
    int rect = SWKJ_BLOCK_SIZE / (openNum / 2);

    //设置大窗口
    DH_IN_SPLIT_OPEN_WINDOW stuOneInputWindow;
    stuOneInputWindow.dwSize = sizeof(DH_IN_SPLIT_OPEN_WINDOW);
    stuOneInputWindow.nChannel = Chn;
    stuOneInputWindow.stuRect.left = 0;
    stuOneInputWindow.stuRect.top = 0;
    stuOneInputWindow.stuRect.right = SWKJ_BLOCK_SIZE - rect;
    stuOneInputWindow.stuRect.bottom = SWKJ_BLOCK_SIZE - rect;
    vecOpenInputPara.emplace_back(stuOneInputWindow);

    DH_OUT_SPLIT_OPEN_WINDOW stuOneOutputWindow;
    stuOneOutputWindow.dwSize = sizeof(DH_OUT_SPLIT_OPEN_WINDOW);
    vecOpenOutputPara.emplace_back(stuOneOutputWindow);

    //设置小窗口（从列开始）
    for(int i = 0; i < openNum / 2; i++){   //遍历行
        for(int j = 0; j < openNum / 2; j++){   //遍历列
            if(i == (openNum / 2 -1) || j == (openNum / 2 -1)){     //判断是否是小窗口
                DH_IN_SPLIT_OPEN_WINDOW stuOneInputWindow;
                stuOneInputWindow.dwSize = sizeof(DH_IN_SPLIT_OPEN_WINDOW);
                stuOneInputWindow.nChannel = Chn;
                stuOneInputWindow.stuRect.left = rect * j;
                stuOneInputWindow.stuRect.top = rect * i;
                stuOneInputWindow.stuRect.right =  rect * (j + 1);
                stuOneInputWindow.stuRect.bottom = rect * (i + 1);
                vecOpenInputPara.emplace_back(stuOneInputWindow);

                DH_OUT_SPLIT_OPEN_WINDOW stuOneOutputWindow;
                stuOneOutputWindow.dwSize = sizeof(DH_OUT_SPLIT_OPEN_WINDOW);
                vecOpenOutputPara.emplace_back(stuOneOutputWindow);
            }
        }
    }
}

Status codeProccess(MonitorWall::codeResponse* request, MonitorWall::empty* response) {
    if(request->code() != 0){
        grpc::Status status(grpc::StatusCode::INTERNAL, "Processing failed");

        auto codeIt = codeMap.find(request->code());
        if(codeIt != codeMap.end()){
            request->set_code_describe(codeMap[request->code()]);
        }else{
            request->set_code_describe("未知错误！");
        }
        return status;
    }else{
        request->set_code_describe(codeMap[request->code()]);
    }
    return Status::OK;
}