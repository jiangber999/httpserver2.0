#include "ChannelMap.h"
#include <string.h>
struct ChannelMap* ChannelMapInit(int size)
{
	struct ChannelMap*Map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
	Map->list = (struct Channel**)malloc(sizeof(struct Channel*) * size);
	Map->size = 0;
	return Map;
}

void ChannelMapClear(struct ChannelMap* map)
{
	if (map != NULL) {
		int i=0;
		for (i = 0; i < map->size; i++) {
			if (map->list[i] != NULL) {
				free(map->list[i]);
				map->list[i] = NULL;
			}		
		}
		free(map->list);
		map->list = NULL;
		map->size = 0;
	}
}

bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize)
{
	if (map->size < newSize) {
		int curSize=map->size;
		while (curSize < newSize) {
			curSize *= 2;
		}
		//À©ÈÝrealloc
		struct Channel**temp =(struct Channel**)realloc(map->list,curSize*unitSize);
		if (temp == NULL) {
			return false;
		}
		map->list = temp;
		memset(&map->list[map->size],0, (curSize-map->size)*unitSize);
		map->size = curSize;


	}
	
	
	
	return true;
}
