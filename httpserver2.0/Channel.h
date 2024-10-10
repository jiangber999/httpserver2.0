#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
typedef int (*handleFunc)(void*arg);
enum FDEvent
{
	TimeOut = 0x01,				   //处理事件超时
	ReadEvent=0x02,
	WriteEvent=0x04
};
struct Channel {
	//文件描述符
	int fd;
	//事件
	int events;
	//回调函数
	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destroyCallback;
	//回调函数的参数
	void* arg;
};
//初始化一个Channel
struct Channel* channelInit(int fd,int event,handleFunc readFunc,handleFunc writeFunc, handleFunc destroyCallback,void *arg);

//修改fd的文件描述符的写事件
void writeEventEnable(struct Channel* channel,bool flag);

//检测fd是否有写事件监听
bool isWriteEventEnable(struct Channel*channel);

