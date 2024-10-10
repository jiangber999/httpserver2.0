#pragma once
#include"Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#define MSG_SEND_AUTO		//�������ַ������ݵķ�ʽ
struct TcpConnection {
	struct Buffer* readBuf;
	struct Buffer* writeBuf;
	struct EventLoop* evLoop;
	struct Channel* channel;
	struct HttpRequest* request;
	struct HttpResponse* response;
	char name[32];
};
//��ʼ��
struct TcpConnection* TcpConnectionInit(struct EventLoop* evLoop, int fd);
//����
int tcpConnectionDestroy(void* arg);


