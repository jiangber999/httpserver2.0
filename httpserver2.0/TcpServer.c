#include "TcpServer.h"
#include <sys/socket.h>
#include "Channel.h"
#include <arpa/inet.h>
#include "TcpConnection.h"
#include <stdlib.h>
struct TcpServer* tcpserverInit(unsigned int port, int threadNum)
{
	struct TcpServer* tcp = (struct TcpServer*)malloc(sizeof(struct TcpServer));
	tcp->mainLoop = EventLoopInit();
	tcp->threadNum = threadNum;
	tcp->pool = threadPoolInit(tcp->mainLoop, tcp->threadNum);
	tcp->listener = listenerInit(port);
	return tcp;
}

struct Listener* listenerInit(unsigned int port)
{
	//���������׽���
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("Socket error");
		return NULL;
	}
	//���ö˿ڸ���
	int opt = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
	if (ret == -1) {
		perror("Setsockopt error");
		return NULL;
	}
	//�󶨵�ַ�ṹ
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);//���Բ�ת�� ���0��С��0��һ����
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		perror("Bind error");
		return NULL;
	}
	//�����������ͬʱ�������ӵ�����
	ret = listen(lfd, 128);
	if (ret == -1) {
		perror("Listen error");
		return NULL;
	}

	
	struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
	listener->fd = lfd;
	listener->port = port;
	return listener;
}

//�����ļ��������ص�����
int acceptConnection(void*arg) {
	struct TcpServer* server = (struct TcpServer*)arg;
	//�Ϳͻ��˽�������
	int cfd = accept(server->listener->fd, NULL, NULL);
	//���̳߳���ȡ��һ�����̵߳ķ�Ӧ��ʵ����ȥ�������lfd
	struct EventLoop*evLoop=takeWorkerEventLoop(server->pool);
	//��cfd�ŵ�TcpConnection�д���
	TcpConnectionInit(evLoop,cfd);

}
void tcpServerRun(struct TcpServer* server)
{
	//�����̳߳�;
	threadPoolRun(server->pool);
	//��Ӽ�������
	struct Channel* channel = channelInit(server->listener->fd, ReadEvent, acceptConnection, NULL,NULL,server);
		//(struct Channel*)malloc(sizeof(struct Channel));
	eventLoopAddTask(server->mainLoop, channel,ADD);
	eventLoopRun(server->mainLoop);




}

