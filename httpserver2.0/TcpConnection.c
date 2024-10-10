#include "TcpConnection.h"
#include <stdlib.h>
int processRead(void *arg) {
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	int count=bufferSocketRead(conn->readBuf, conn->channel->fd);
	if (count > 0) {
		//���յ���http���󣬽���http����
		
#ifdef MSG_SEND_AUTO

		writeEventEnable(conn->channel, true);
		eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
#endif
		bool flag = parseHttpRequest(conn->request, conn->readBuf, conn->response, conn->writeBuf, conn->channel->fd);
		if (!flag) {
		//����ʧ�ܣ��ظ�һ���򵥵�html
		char* errMsg = "http/1.1 400 Bad Request\r\n\r\n";
		bufferAppendString(conn->writeBuf, errMsg);
		//�Ͽ�����
		}

	}

 #ifndef MSG_SEND_AUTO

	//�Ͽ�����
	eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
	return 0;

}
int processwrite(void *arg) {
	struct TcpConnection *conn = (struct TcpConnection*)arg;
	//��������
	int count=bufferSendData(conn->writeBuf, conn->channel->fd);
	if (count > 0) {
		//�ж������Ƿ�ȫ�����ͳ�ȥ��
		if (bufferReadableSize(conn->writeBuf) == 0) {
			//1.���ټ��д�¼�--�޸�Channel�еı����¼�
			writeEventEnable(conn->channel, false);
			//2.�޸�dispatcher���ļ���--�������ڵ�
			eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
			//3.ɾ������ڵ�
			eventLoopAddTask(conn->evLoop, conn->channel, DELETE);

		}
	}
	return 0;
}
struct TcpConnection* TcpConnectionInit(struct EventLoop*evLoop,int fd)
{
	struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
	conn->readBuf=bufferInit(10240);
	conn->writeBuf = bufferInit(10240);
	conn->evLoop = evLoop;
	conn->channel = channelInit(fd, ReadEvent, processRead, processwrite,tcpConnectionDestroy,conn);
	sprintf(conn->name, "connection-%d", fd);
	eventLoopAddTask(evLoop, conn->channel, ADD);
	conn->request = httpRequestInit();
	conn->response = httpResponseInit();
	return conn;
}

int tcpConnectionDestroy(void*arg)
{
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	if (conn != NULL) {
		if (conn->readBuf && bufferReadableSize(conn->readBuf)==0
			&& conn->writeBuf && bufferWriteableSize(conn->writeBuf)==0) {
			destroyChannel(conn->evLoop, conn->channel);
			httpRequestDestory(conn->request);
			httpResponseDestory(conn->response);
			bufferDestroy(conn->readBuf);
			bufferDestroy(conn->writeBuf);
			free(conn);
		}
		
	}
	return 0;
}
