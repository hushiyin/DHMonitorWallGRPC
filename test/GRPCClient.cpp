#include <grpcpp/grpcpp.h>
#include "MonitorWall.grpc.pb.h"
#include <iostream>
#include <unistd.h> // for sleep

class MonitorWallClient {
public:
    MonitorWallClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(MonitorWall::SWKJMonitorWall::NewStub(channel)) {}

    void ClientLogin(const std::string& decoderIp, uint32_t decoderPort, const std::string& username, const std::string& password)
    {
        // 创建请求和响应对象
        MonitorWall::loginRequest request;
        MonitorWall::codeResponse response;

        // 填充请求数据
        request.set_decoder_ip(decoderIp);
        request.set_decoder_port(decoderPort);
        request.set_decoder_username(username);
        request.set_decoder_pwd(password);

        // 发起 RPC 调用
        grpc::ClientContext context;
        grpc::Status status = stub_->login(&context, request, &response);

        // 处理响应
        if (status.ok()) {
            std::cout << "Login successful!" << std::endl;
        } else {
            std::cerr << "Login failed: " << status.error_message() << std::endl;
        }
    }

    int ClientGetTVNum(const std::string& decoderIp, uint32_t decoderPort, const std::string& username, const std::string& password)
    {
        MonitorWall::loginRequest request;
        MonitorWall::decoderOutTVNumResponse response;

        request.set_decoder_ip(decoderIp);
        request.set_decoder_port(decoderPort);
        request.set_decoder_username(username);
        request.set_decoder_pwd(password);

        grpc::ClientContext context;
        grpc::Status status = stub_->getTVNum(&context, request, &response);

        for(int i = 0; i < response.tv_num(); i++)
        {
            std::cout<<"out tv name: "<<response.tv_info(i).tv_name()<<"    "<<"channel id: "<<response.tv_info(i).cnannel_id()<<std::endl;
        }

        // 处理响应
        if (status.ok()) {
            std::cout << "get tv nums successful!" << std::endl;
            return response.tv_num();
        }
        else {
            std::cerr << "get tv nums failed: " << status.error_message() << std::endl;
            return -1;
        }
    }

    void ClientCreateWall(int blockNums, const std::vector<MonitorWall::blockMes>& blocks, MonitorWall::loginRequest* res)
    {
        // 创建请求
        MonitorWall::wallConfigRequest request;
        request.set_block_num(blockNums);
        MonitorWall::tvMesResponse response;

        // 添加 blocks 到请求
        for (const auto& block : blocks) {
            *request.add_block() = block;
        }

        request.mutable_login_res()->CopyFrom(*res);

        grpc::ClientContext context;
        grpc::Status status = stub_->createWall(&context, request, &response);
        sleep(1);
        for(auto& tvMessage: response.tv_list())
        {
            std::cout<<tvMessage.out_tv_id()<<"\n"
                     <<tvMessage.is_bind()<<"\n"
                     <<tvMessage.block_line()<<"\n"
                     <<tvMessage.block_col()<<std::endl;
            std::cout<<std::endl;
        }

        if (status.ok()) {
            std::cout << "create monitorwall successful!" << std::endl;
        }
        else {
            std::cerr << "create monitorwall failed: " << status.error_message() << std::endl;
        }
    }

    void ClientSetSource(MonitorWall::loginRequest* res, MonitorWall::sourceCommonRequest* source)
    {
        //创建请求
        MonitorWall::setSourceRequest request;
        MonitorWall::devMes dev_mes;
        //创建返回
        MonitorWall::codeResponse response;
        dev_mes.set_dev_ip("192.168.17.67");
        dev_mes.set_dev_port(37777);
        dev_mes.set_dev_user("admin");
        dev_mes.set_dev_pwd("admin");
        dev_mes.set_dev_channel_id(0);


        request.mutable_login_res()->CopyFrom(*res);
        request.mutable_source_res()->CopyFrom(*source);
        request.mutable_dev_mes()->CopyFrom(dev_mes);

        grpc::ClientContext context;
        grpc::Status status = stub_->setVideoSource(&context, request, &response);

        if (status.ok()) {
            std::cout << "set source successful!" << std::endl;
        }
        else {
            std::cerr << "set source failed: " << status.error_message() << std::endl;
        }
    }

    void ClientDelSource(MonitorWall::loginRequest* res, MonitorWall::sourceCommonRequest* source)
    {
        MonitorWall::delSourceRequset delResquest;
        delResquest.set_del_mode(1);
        delResquest.mutable_login_res()->CopyFrom(*res);
        delResquest.mutable_source_res()->CopyFrom(*source);
        MonitorWall::codeResponse response;

        grpc::ClientContext context;
        grpc::Status status = stub_->delVideoSource(&context, delResquest, &response);

        std::cout<<"response.code(): "<<response.code()<<std::endl;
        std::cout<<"code_describe(): "<<response.code_describe()<<std::endl;

        if (status.ok()) {
            std::cout << "del source successful!" << std::endl;
        }
        else {
            std::cerr << "del source failed: " << status.error_message() << std::endl;
        }
    }

    void ClientSearchDev(std::shared_ptr<MonitorWall::searchDevRequest> response)
    {
        MonitorWall::empty request;

        grpc::ClientContext context;
        grpc::Status status = stub_->searchDev(&context, request, response.get());

        if (status.ok()) {
            std::cout << "del source successful!" << std::endl;
        }
        else {
            std::cerr << "del source failed: " << status.error_message() << std::endl;
        }
    }

    void ClientGetDispaly(MonitorWall::loginRequest res)
    {
        MonitorWall::allWindowDisplayMesResponse all_win_mes;
        grpc::ClientContext context;
        grpc::Status status = stub_->getDisplayMes(&context, res, &all_win_mes);

        for(auto& mes: all_win_mes.one_win_mes())
        {
            std::cout<<"out_tv_id: "<<mes.out_tv_id()<<"\n"
                     <<"win_num: "<<mes.win_num()<<"\n"
                     <<"is bind: "<<mes.is_bind()<<"\n"
                     <<"block line: "<<mes.block_line()<<"\n"
                     <<"block col: "<<mes.block_col()<<std::endl;
            for(auto& cam: mes.cam_mes()){
                std::cout<<"camera_name: "<<cam.camera_name()<<"\n"
                         <<"stream_type: "<<cam.stream_type()<<"\n"
                         <<"camera_ip: "<<cam.camera_ip()<<"\n"
                         <<"camera_channel: "<<cam.camera_channel()<<"\n"
                         <<"win_id: "<<cam.win_id()<<std::endl;
            }
            std::cout<<std::endl;
        }

        if (status.ok()) {
            std::cout << "query dispaly message successful!" << std::endl;
        }
        else {
            std::cerr << "query dispaly message failed: " << status.error_message() << std::endl;
        }
    }

    void ClientGetTVStatus(MonitorWall::loginRequest res)
    {
        MonitorWall::tvMesResponse response;

        grpc::ClientContext context;
        grpc::Status status = stub_->getTVStatus(&context, res, &response);

        for(auto& list: response.tv_list())
        {
            std::cout<<"out_tv_id: "<<list.out_tv_id()<<"\n"
                     <<"is_bind: "<<list.is_bind()<<"\n"
                     <<"block_line: "<<list.block_line()<<"\n"
                     <<"block_col: "<<list.block_col()<<std::endl;
            std::cout<<std::endl;
        }

        if (status.ok()) {
        std::cout << "get tv status successful!" << std::endl;
        }
        else {
            std::cerr << "get tv status failed: " << status.error_message() << std::endl;
        }
    }

    void ClientSplitWindow(MonitorWall::loginRequest res, int openNum){
        MonitorWall::splitWindowRequest request;
        request.set_spilt_num(openNum);
        request.set_tv_id(0);
        request.mutable_login_res()->CopyFrom(res);

        MonitorWall::codeResponse response;

        grpc::ClientContext context;
        grpc::Status status = stub_->splitWindow(&context, request, &response);

        std::cout<<"code: "<<response.code()<<"\n"
                 <<"code_describe: "<<response.code_describe()<<std::endl;

        if (status.ok()) {
            std::cout << "splitWindow successful!" << std::endl;
        }
        else {
            std::cerr << "splitWindow failed: " << status.error_message() << std::endl;
        }
    }

    void ClientGetStreamInfo(MonitorWall::loginRequest res){
        MonitorWall::windowSreamInfoResponse streamResponse;

        grpc::ClientContext context;
        grpc::Status status = stub_->queryWindowStreamInfo(&context, res, &streamResponse);
        for(auto stream: streamResponse.block_stream_info())
        {
            std::cout<<"tv_id: "<<stream.tv_id()<<std::endl;
            for(auto win_stream: stream.win_stream_info())
            {
                std::cout<<"window_id: "<<win_stream.window_id()<<"\n"
                     <<"bit_rate: "<< win_stream.bit_rate()<<"\n"
                     <<"frame_rate: "<< win_stream.frame_rate()<<"\n"
                     <<"resolution: "<< win_stream.resolution()<<"\n"
                     <<"compression: "<< win_stream.compression()<<std::endl;
            }
            std::cout<<std::endl;
        }

        if (status.ok()) {
            std::cout << "ClientGetStreamInfo successful!" << std::endl;
        }
        else {
            std::cerr << "ClientGetStreamInfo failed: " << status.error_message() << std::endl;
        }
    }


private:
    std::unique_ptr<MonitorWall::SWKJMonitorWall::Stub> stub_;
};

int main(int argc, char** argv) {
    std::string decoderIp = "192.168.17.108";
    uint32_t decoderPort = 37777;
    std::string username = "admin";
    std::string password = "swkj@2024";

    // 创建客户端
    MonitorWallClient client(grpc::CreateChannel("192.168.17.46:50051", grpc::InsecureChannelCredentials()));

    // client.ClientLogin(decoderIp, decoderPort, username, password);

    //创建电视墙
    std::vector<MonitorWall::blockMes> block_vec;
    // MonitorWall::blockMes block1;
    // block1.set_bind_line(0);
    // block1.set_bind_col(0);
    // block1.set_bind_ch(0);
    // MonitorWall::blockMes block2;
    // block2.set_bind_line(2);
    // block2.set_bind_col(1);
    // block2.set_bind_ch(1);
    // block_vec.emplace_back(block1);
    // block_vec.emplace_back(block2);

    MonitorWall::loginRequest res;
    res.set_decoder_ip(decoderIp);
    res.set_decoder_port(decoderPort);
    res.set_decoder_username(username);
    res.set_decoder_pwd(password);

    // client.ClientCreateWall(0, block_vec, &res);

    //绑定信号源
    MonitorWall::sourceCommonRequest source;
    source.set_channel_id(0);
    source.set_window_id(0);
    // client.ClientSetSource(&res, &source);

    //解绑信号源
    MonitorWall::sourceCommonRequest sourceRequest;
    sourceRequest.set_channel_id(0);
    sourceRequest.set_window_id(0);
    client.ClientDelSource(&res, &sourceRequest);

    //获取电视墙配置
    // client.ClientCreateWall(res);

    //显示信息
    // client.ClientGetDispaly(res);

    //查询输出口的绑定状态
    // client.ClientGetTVStatus(res);

    //分屏
    // client.ClientSplitWindow(res, 4);

    //查询流信息
    // .ClientGetStreamInfo(res);

    // client.ClientGetTVNum(decoderIp, decoderPort, username, password);
    return 0;
}