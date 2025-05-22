#pragma once
#include<iostream>

#define REALPLAY	//控制实时播放功能
// #define IS_IMPL		//已经实现的功能

#define MAX_BUF_LEN (2 * 1024 * 1024)	//获取配置的缓冲区大小
#define WAIT_TIME			5000

//用于绑定输出口，所有值都是从0开始
typedef struct BindOutputTV{
    int BindLine;   //block行
    int BindCol;   //block列
    int BindCh;     //需要绑定的输出口
};

void SaveJsonToFile(const std::string& filename, const char* jsonData);    //保存文件到Json