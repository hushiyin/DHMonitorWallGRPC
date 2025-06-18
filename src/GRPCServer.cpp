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
    Status login(ServerContext* context, const loginRequest* request, MonitorWall::codeResponse* response) override {
        dhDecoderPtr->code = 0;
        strcpy(dhDecoderPtr->m_decoderIp, request->decoder_ip().c_str());
        strcpy(dhDecoderPtr->m_decoderName, request->decoder_username().c_str());
        strcpy(dhDecoderPtr->m_decoderPwd, request->decoder_pwd().c_str());
        dhDecoderPtr->m_decoderPort = request->decoder_port();

        dhDecoderPtr->longinDev();
        saveLoginInfo(request->decoder_ip(), request->decoder_port(), request->decoder_username(), request->decoder_pwd());

        response->set_code(dhDecoderPtr->code);

        codeProccess(response, emptyRes.get());
        return Status::OK;
    }

    Status getTVNum(ServerContext* context, const loginRequest* request, decoderOutTVNumResponse* response) override {
        dhDecoderPtr->code = 0;
        checkLogin(context, request, const_cast<MonitorWall::codeResponse*>(&response->code_info()));

        response->set_tv_num(dhDecoderPtr->queryDecoderInfo());
        for(int i = 0; i < response->tv_num(); i++){
            MonitorWall::outTV out_tv;
            out_tv.set_cnannel_id(i);
            std::string name = "hdmi" + std::to_string(i);
            out_tv.set_tv_name(name);
            *response->add_tv_info() = out_tv;
        }

        auto pcodeMes = std::make_shared<MonitorWall::codeResponse>();
        pcodeMes->set_code(dhDecoderPtr->code);
        response->mutable_code_info()->CopyFrom(*pcodeMes.get());
        codeProccess(const_cast<MonitorWall::codeResponse*>(&response->code_info()), emptyRes.get());
        return Status::OK;
    }

    Status createWall(ServerContext* context, const wallConfigRequest* request, MonitorWall::tvMesResponse* response) override {
        dhDecoderPtr->code = 0;
        std::vector<BindOutputTV> vecBlock;     //存储block的坐标和要绑定的输出口通道
        checkLogin(context, &request->login_res(), const_cast<MonitorWall::codeResponse*>(&response->code_info()));

        //根据block坐标绑定输出口
        for(auto stuBlock: request->block()){
            BindOutputTV bind_out_tv;
            bind_out_tv.BindLine = stuBlock.bind_line();
            bind_out_tv.BindCol = stuBlock.bind_col();
            bind_out_tv.BindCh = stuBlock.bind_ch();

            vecBlock.emplace_back(bind_out_tv);
        }

        dhDecoderPtr->setMonitorWallCfg(request->block_num(), vecBlock, response);

        auto pcodeMes = std::make_shared<MonitorWall::codeResponse>();
        pcodeMes->set_code(dhDecoderPtr->code);
        response->mutable_code_info()->CopyFrom(*pcodeMes.get());
        codeProccess(const_cast<MonitorWall::codeResponse*>(&response->code_info()), emptyRes.get());
        return Status::OK;
    }

    Status setVideoSource(ServerContext* context, const MonitorWall::setSourceRequest* request, MonitorWall::codeResponse* response) override {
        dhDecoderPtr->code = 0;
        checkLogin(context, &request->login_res(), response);

        dhDecoderPtr->setSource(request->source_res().channel_id(), request->source_res().window_id(), request->dev_mes().dev_ip(), request->dev_mes().dev_port(), 
                                request->dev_mes().dev_user(), request->dev_mes().dev_pwd(), request->dev_mes().dev_channel_id());

        response->set_code(dhDecoderPtr->code);
        codeProccess(response, emptyRes.get());
        return Status::OK;
    }

    Status delVideoSource(ServerContext* context, const MonitorWall::delSourceRequset* request, MonitorWall::codeResponse* response) override {
        dhDecoderPtr->code = 0;
        checkLogin(context, &request->login_res(), response);

        switch(request->del_mode()){
            case 1: //单个解绑
                dhDecoderPtr->delSource(request->source_res().channel_id(), request->source_res().window_id());
                break;
            case 2: //解绑某个块
                dhDecoderPtr->delSource(request->source_res().channel_id());
                break;
            case 3: //解绑所有
                dhDecoderPtr->delSource();
                break;
        }

        response->set_code(dhDecoderPtr->code);
        codeProccess(response, emptyRes.get());
        return Status::OK;
    }

    Status searchDev(ServerContext* context, const MonitorWall::empty* request, MonitorWall::searchDevRequest* response) override {
        dhDecoderPtr->code = 0;
        dhDecoderPtr->asyncSearchDev(response);

        auto pcodeMes = std::make_shared<MonitorWall::codeResponse>();
        pcodeMes->set_code(dhDecoderPtr->code);
        response->mutable_code_info()->CopyFrom(*pcodeMes.get());
        codeProccess(const_cast<MonitorWall::codeResponse*>(&response->code_info()), emptyRes.get());
        return Status::OK;
    }

    Status getDisplayMes(ServerContext* context, const MonitorWall::loginRequest* request, MonitorWall::allWindowDisplayMesResponse* response) override {
        dhDecoderPtr->code = 0;
        checkLogin(context, request, const_cast<MonitorWall::codeResponse*>(&response->code_info()));

        int tv_num = dhDecoderPtr->queryDecoderInfo();
        std::vector<wallConfig> vec_wall = dhDecoderPtr->getMonitorWallCfg();		//查询到的块信息
        std::vector<int> tv_flag;       //输出口是否绑定的标志

        for(int i = 0; i < tv_num; i++){
            for(auto& queryMes: vec_wall){
                //判断该通道是否绑定
                if(queryMes.TVID == i){
                    tv_flag.emplace_back(i);    //存储已绑定的通道号
                }
            }
        }

        for(int j = 0; j < tv_num; j++){
            auto it = std::find(tv_flag.begin(), tv_flag.end(), j);
            //该输出口已绑定,那就能查询窗口信息
            if(it != tv_flag.end()){
                dhDecoderPtr->querySource(j, response);     //将窗口的信息返回
            }
        }

        auto pcodeMes = std::make_shared<MonitorWall::codeResponse>();
        pcodeMes->set_code(dhDecoderPtr->code);
        response->mutable_code_info()->CopyFrom(*pcodeMes.get());
        codeProccess(const_cast<MonitorWall::codeResponse*>(&response->code_info()), emptyRes.get());
        return Status::OK;
    }

    Status getTVStatus(ServerContext* context, const MonitorWall::loginRequest* request, MonitorWall::tvMesResponse* response) override {
        dhDecoderPtr->code = 0;
        checkLogin(context, request, const_cast<MonitorWall::codeResponse*>(&response->code_info()));
        std::vector<wallConfig> vec_wall = dhDecoderPtr->getMonitorWallCfg();

        if(vec_wall.size() != 0){
            for(auto& queryMes: vec_wall){
                MonitorWall::tvList tv_list;
                tv_list.set_out_tv_id(queryMes.TVID);
                tv_list.set_is_bind(true);
                tv_list.set_block_line(queryMes.b_line);
                tv_list.set_block_col(queryMes.b_col);

                *response->add_tv_list() = tv_list;
            }
        }

        auto pcodeMes = std::make_shared<MonitorWall::codeResponse>();
        pcodeMes->set_code(dhDecoderPtr->code);
        response->mutable_code_info()->CopyFrom(*pcodeMes.get());
        codeProccess(const_cast<MonitorWall::codeResponse*>(&response->code_info()), emptyRes.get());
        return Status::OK;
    }

    Status splitWindow(ServerContext* context, const MonitorWall::splitWindowRequest* request, MonitorWall::codeResponse* response) override {
        dhDecoderPtr->code = 0;
        checkLogin(context, &request->login_res(), response);

        dhDecoderPtr->setSplitMode(request->tv_id(), request->spilt_num());
        response->set_code(dhDecoderPtr->code);

        //错误处理
        codeProccess(response, emptyRes.get());
        return Status::OK;
    }

    Status queryWindowStreamInfo(ServerContext* context, const MonitorWall::loginRequest* request, MonitorWall::windowSreamInfoResponse* response) override {
        dhDecoderPtr->code = 0;
        checkLogin(context, request, const_cast<MonitorWall::codeResponse*>(&response->code_info()));

        dhDecoderPtr->getStreamInfo();
        if(dhDecoderPtr->m_vBlock.size() != 0)
        {
            for(auto blockMes: dhDecoderPtr->m_vBlock)
            {
                if(blockMes->m_vwinInfo.size() > 0)
                {
                    MonitorWall::blockStreamInfo block_stream_mes;
                    block_stream_mes.set_tv_id(blockMes->m_channelID);
                    for(auto streamInfo: blockMes->m_vwinInfo)
                    {
                        if(streamInfo->m_bitRate > 0){
                            MonitorWall::windowStreamInfo win_stream_info;
                            win_stream_info.set_window_id(streamInfo->m_windowID);
                            win_stream_info.set_bit_rate(streamInfo->m_bitRate);
                            win_stream_info.set_frame_rate(streamInfo->m_frameRate);
                            win_stream_info.set_resolution(resolution[streamInfo->m_resolution]);
                            win_stream_info.set_compression(encoderCompression[streamInfo->m_compression]);

                            *block_stream_mes.add_win_stream_info() = win_stream_info;
                        }
                    }
                    *response->add_block_stream_info() = block_stream_mes;
                }
            }
        }

        codeProccess(const_cast<MonitorWall::codeResponse*>(&response->code_info()), emptyRes.get());
        return Status::OK;
    }


private:
    std::shared_ptr<MonitorWall::empty> emptyRes;

    Status checkLogin(ServerContext* context, const loginRequest* request, MonitorWall::codeResponse* response) override {
        if(!queryLoginInfo(request->decoder_ip(), request->decoder_port(), request->decoder_username(), request->decoder_pwd())){
            login(context, request, response);
            saveLoginInfo(request->decoder_ip(), request->decoder_port(), request->decoder_username(), request->decoder_pwd());
        }
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