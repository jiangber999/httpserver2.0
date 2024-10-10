#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
typedef int (*handleFunc)(void*arg);
enum FDEvent
{
	TimeOut = 0x01,				   //�����¼���ʱ
	ReadEvent=0x02,
	WriteEvent=0x04
};
struct Channel {
	//�ļ�������
	int fd;
	//�¼�
	int events;
	//�ص�����
	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destroyCallback;
	//�ص������Ĳ���
	void* arg;
};
//��ʼ��һ��Channel
struct Channel* channelInit(int fd,int event,handleFunc readFunc,handleFunc writeFunc, handleFunc destroyCallback,void *arg);

//�޸�fd���ļ���������д�¼�
void writeEventEnable(struct Channel* channel,bool flag);

//���fd�Ƿ���д�¼�����
bool isWriteEventEnable(struct Channel*channel);

