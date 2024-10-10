#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "Channel.h"
#include <stdlib.h>
struct ChannelMap {
	//记录指针元素个数
	int size;
	struct Channel** list;
};
//初始化Map
struct ChannelMap* ChannelMapInit(int size);
//清空map
void ChannelMapClear(struct ChannelMap*Map);
//重新分配内存空间
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);