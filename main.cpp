

#include "heatmap.h"

#include <time.h>

int main()
{
	char string[256];
	srand(time(null));

	HEAT_MAP HeatMap(1);

	for(int i = 0; i < 100000; i++) {
		HeatMap.AddValue(GOLD_DROP, 1, rand() % 10000, rand() % 10000);
	}

	for(int i = 0; i < 100000; i++) {
		HeatMap.AddValue(MONSTER_KILL, 1, rand() % 10000, rand() % 10000);
	}

	HeatMap.Draw(GOLD_DROP,0,0,30,30);
	HeatMap.Draw(MONSTER_KILL,0,0,30,30);
	sprintf_s(string, 256, "Monster Kill Value From 22-23/22-23: %1.0f\n", HeatMap.GetValue(MONSTER_KILL,22,23,22,23));
	OutputDebugString(string);

	int BufferSize = 20*1024*1024;
	void* Buffer = Malloc(BufferSize, __FILE__, __LINE__);

	HeatMap.Serialize(Buffer, BufferSize);
	HeatMap.Deserialize(Buffer, BufferSize);

	HeatMap.Draw(GOLD_DROP,0,0,30,30);
	HeatMap.Draw(MONSTER_KILL,0,0,30,30);

	HEAT_MAP HeatMap2(10);
	HeatMap2.Deserialize(Buffer, BufferSize);
	HeatMap2.Draw(GOLD_DROP,0,0,30,30);
	HeatMap2.Draw(MONSTER_KILL,0,0,30,30);

}