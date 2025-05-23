#include <grpcpp/grpcpp.h>
#include "MonitorWall.grpc.pb.h"
#include <iostream>

class MonitorWallClient {
public:
    MonitorWallClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(MonitorWall::SWKJMonitorWall::NewStub(channel)) {}

    void ClientLogin(const std::string& decoderIp, uint32_t decoderPort, const std::string& username, const std::string& password)
    {
        // 创建请求和响应对象
        MonitorWall::loginRequest request;
        MonitorWall::empty response;

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

    void ClientCreateWall(int line, int colunm, const std::vector<MonitorWall::blockMes>& blocks, MonitorWall::loginRequest* res)
    {
        // 创建请求
        MonitorWall::wallConfigRequest request;
        request.set_line(line);
        request.set_colunm(colunm);
        MonitorWall::empty response;

        // 添加 blocks 到请求
        for (const auto& block : blocks) {
            *request.add_block() = block;
        }

        request.mutable_login_res()->CopyFrom(*res);

        grpc::ClientContext context;
        grpc::Status status = stub_->createWall(&context, request, &response);

        if (status.ok()) {
            std::cout << "create monitorwall successful!" << std::endl;
        }
        else {
            std::cerr << "create monitorwall failed: " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<MonitorWall::SWKJMonitorWall::Stub> stub_;
};

int main(int argc, char** argv) {
    // 检查命令行参数
    if (argc < 5)
    {
        std::cerr << "Usage: client <decoderIp> <decoderPort> <username> <password>" << std::endl;
        return 1;
    }

    std::string decoderIp = argv[1];
    uint32_t decoderPort = std::stoi(argv[2]);
    std::string username = argv[3];
    std::string password = argv[4];

    // 创建客户端
    MonitorWallClient client(grpc::CreateChannel("192.168.17.63:50051", grpc::InsecureChannelCredentials()));

    // 调用登录方法
    // client.ClientLogin(decoderIp, decoderPort, username, password);
    int decoderTVNum = client.ClientGetTVNum(decoderIp, decoderPort, username, password);
    std::cout<<decoderTVNum<<std::endl;

    //创建电视墙
    std::vector<MonitorWall::blockMes> block_vec;
    MonitorWall::blockMes block1;
    block1.set_bind_line(0);
    block1.set_bind_col(0);
    block1.set_bind_ch(1);
    MonitorWall::blockMes block2;
    block2.set_bind_line(0);
    block2.set_bind_col(1);
    block2.set_bind_ch(0);
    block_vec.emplace_back(block1);
    block_vec.emplace_back(block2);

    MonitorWall::loginRequest res;
    res.set_decoder_ip(decoderIp);
    res.set_decoder_port(decoderPort);
    res.set_decoder_username(username);
    res.set_decoder_pwd(password);

    client.ClientCreateWall(1, 2, block_vec, &res);

    return 0;
}