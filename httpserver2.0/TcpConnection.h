#pragma once
#include"Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#define MSG_SEND_AUTO		//控制两种发送数据的方式
struct TcpConnection {
	struct Buffer* readBuf;
	struct Buffer* writeBuf;
	struct EventLoop* evLoop;
	struct Channel* channel;
	struct HttpRequest* request;
	struct HttpResponse* response;
	char name[32];
};
//初始化
struct TcpConnection* TcpConnectionInit(struct EventLoop* evLoop, int fd);
//销毁
int tcpConnectionDestroy(void* arg);


