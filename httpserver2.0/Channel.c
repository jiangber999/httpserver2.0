#include "Channel.h"
#include <stdlib.h>
struct Channel* channelInit(int fd, int event, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyCallback, void* arg)
{
	struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
	channel->fd = fd;
	channel->events = event;
	channel->readCallback = readFunc;
	channel->writeCallback = writeFunc;
	channel->destroyCallback = destroyCallback;
	channel->arg = arg;
	return channel;
}

void writeEventEnable(struct Channel* channel,bool flag)
{
	if (flag) {
		channel->events |=  WriteEvent;
	}
	else {
		channel->events &= ~WriteEvent;
	}
}

bool isWriteEventEnable(struct Channel* channel)
{
	return channel->events&WriteEvent;//不用考虑读事件，0给&没了
}
