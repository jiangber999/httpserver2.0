#pragma once
#include "EventLoop.h"
#include "Channel.h"
struct EventLoop;
struct Dispatcher {
	//初始化epoll、poll、select需要的数据块
	void* (*init)();
	//添加文件描述符
	int (*add)(struct Channel* channel, struct EventLoop* evLoop);
	//删除文件描述符
	int (*remove)(struct Channel* channel, struct EventLoop* evLoop);
	//修改文件描述符
	int (*modify)(struct Channel* channel, struct EventLoop* evLoop);
	//监测事件
	int (*dispatch)(struct EventLoop* evLoop,int timeout);//单位s
	//清除数据（关闭文件描述符或者释放内存）
	int (*clear)(struct EventLoop* evLoop);
};