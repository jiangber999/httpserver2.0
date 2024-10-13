#pragma once
#include "EventLoop.h"
#include <sys/socket.h>
struct Buffer {
	//ָ���ڴ��ָ��
	char* data;
	int writePos;
	int readPos;
	int capacity;
};
//��ʼ��
struct Buffer* bufferInit(int size);
//����
void bufferDestroy(struct Buffer* buffer);
//����
void bufferExtendRoom(struct Buffer* buffer, int size);
//�õ�ʣ���д���ڴ�����
int bufferWriteableSize(struct Buffer* buffer);
//�õ�ʣ��ɶ����ڴ�����
int bufferReadableSize(struct Buffer* buffer);
//д�ڴ� 1.ֱ��д2.�����׽�������
int bufferAppendData(struct Buffer*buffer,const char*data,int size);
int bufferAppendString(struct Buffer*buffer,const char*data);
int bufferSocketRead(struct Buffer* buffer, int fd);
//����������
char* bufferFindCRLF(struct Buffer* buffer);
//��������
int bufferSendData(struct Buffer* buffer, int socket);
