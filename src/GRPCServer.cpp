#include <iostream>
#include <memory>
#include <string>

#include "DHDecoder.h"
#include "dhnetsdk.h"
#include "Utils.h"

#include <grpcpp/grpcpp.h>
#include "MonitorWall.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
//server
using MonitorWall::SWKJMonitorWall;
//message
using MonitorWall::empty;
using MonitorWall::loginRequest;
using MonitorWall::decoderOutTVNumResponse;
using MonitorWall::blockMes;
using MonitorWall::wallConfigRequest;

class SWKJMonitorWallServiceImpl final : public SWKJMonitorWall::Service 
{
    std::shared_ptr<DHDecoder> dhDecoderPtr = std::make_shared<DHDecoder>();
public:
    Status login(ServerContext* context, const loginRequest* request, empty* response) override
    {
        strcpy(dhDecoderPtr->m_decoderIp, request->decoder_ip().c_str());
        strcpy(dhDecoderPtr->m_decoderName, request->decoder_username().c_str());
        strcpy(dhDecoderPtr->m_decoderPwd, request->decoder_pwd().c_str());
        dhDecoderPtr->m_decoderPort = request->decoder_port();

        dhDecoderPtr->longinDev();
        saveLoginInfo(request->decoder_ip(), request->decoder_port(), request->decoder_username(), request->decoder_pwd());
        return Status::OK;
    }

    Status getTVNum(ServerContext* context, const loginRequest* request, decoderOutTVNumResponse* response) override
    {
        if(!queryLoginInfo(request->decoder_ip(), request->decoder_port(), request->decoder_username(), request->decoder_pwd()))
        {
            empty res;
            login(context, request, &res);
            saveLoginInfo(request->decoder_ip(), request->decoder_port(), request->decoder_username(), request->decoder_pwd());
        }

        response->set_tv_num(dhDecoderPtr->queryDecoderInfo());
        return Status::OK;
    }

    Status createWall(ServerContext* context, const wallConfigRequest* request, empty* response) override
    {
        std::vector<BindOutputTV> vecBlock;     //存储block的坐标和要绑定的输出口通道

        //查询登录信息
        if(!queryLoginInfo(request->login_res().decoder_ip(), request->login_res().decoder_port(), 
                                                    request->login_res().decoder_username(), request->login_res().decoder_pwd()))
        {
            empty res;
            login(context, &request->login_res(), &res);
            saveLoginInfo(request->login_res().decoder_ip(), request->login_res().decoder_port(), 
                                                    request->login_res().decoder_username(), request->login_res().decoder_pwd());
        }

        //根据block坐标绑定输出口
        int block_num = request->line()*request->colunm();
        for(auto stuBlock: request->block()){
            BindOutputTV bind_out_tv;
            bind_out_tv.BindLine = stuBlock.bind_line();
            bind_out_tv.BindCol = stuBlock.bind_col();
            bind_out_tv.BindCh = stuBlock.bind_ch();

            vecBlock.emplace_back(bind_out_tv);
        }

        dhDecoderPtr->setMonitorWallCfg(request->line(), request->colunm(), vecBlock);
        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    SWKJMonitorWallServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait(); // 等待请求
}

int main() {
    RunServer();
    return 0;
}