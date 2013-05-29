

#include "heatmap.h"

#include <time.h>

int main()
{
	char string[256];
	srand(time(null));

	HEAT_MAP HeatMap(1);

	// 20 million random drops from 0,0 to 10000,10000
	for(int i = 0; i < 10000000; i++) {
		HeatMap.AddValue(GOLD_DROP, 1, rand() % 10000, rand() % 10000);
	}
	for(int i = 0; i < 10000000; i++) {
		HeatMap.AddValue(MONSTER_KILL, 1, rand() % 10000, rand() % 10000);
	}

	// 2 million random drops from 0,0 to 10,10
	for(int i = 0; i < 1000000; i++) {
		HeatMap.AddValue(GOLD_DROP, 1, rand() % 10, rand() % 10);
	}
	for(int i = 0; i < 1000000; i++) {
		HeatMap.AddValue(MONSTER_KILL, 1, rand() % 10, rand() % 10);
	}

	HeatMap.Draw(GOLD_DROP,0,0,30,30);
	HeatMap.Draw(MONSTER_KILL,0,0,30,30);
	OutputDebugString(string);

	int BufferSize = 500*1024*1024;
	void* Buffer = Malloc(BufferSize, __FILE__, __LINE__);

	HeatMap.Serialize(Buffer, BufferSize);
	HeatMap.Deserialize(Buffer, BufferSize);

	HeatMap.Draw(GOLD_DROP,0,0,30,30);
	HeatMap.Draw(MONSTER_KILL,0,0,30,30);

	HEAT_MAP HeatMap2(10);
	HeatMap2.Deserialize(Buffer, BufferSize);
	HeatMap2.Draw(GOLD_DROP,0,0,300,300);
	HeatMap2.Draw(MONSTER_KILL,0,0,300,300);

	HEAT_MAP HeatMap3(100);
	HeatMap3.Deserialize(Buffer, BufferSize);
	HeatMap3.Draw(GOLD_DROP,0,0,3000,3000);
	HeatMap3.Draw(MONSTER_KILL,0,0,3000,3000);
}