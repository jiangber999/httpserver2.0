#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "Channel.h"
#include <stdlib.h>
struct ChannelMap {
	//��¼ָ��Ԫ�ظ���
	int size;
	struct Channel** list;
};
//��ʼ��Map
struct ChannelMap* ChannelMapInit(int size);
//���map
void ChannelMapClear(struct ChannelMap*Map);
//���·����ڴ�ռ�
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);