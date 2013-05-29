//******************************************************************************
// Heat Map
// --------
//
// @author Doug Frazer
// May 2013
//******************************************************************************

#ifndef __HEAT_MAP_H__
#define __HEAT_MAP_H__

#include "common_includes.h"

#include "pool_heap.h"
#include "hashmap.h"

// Represents values that we would be interested in counting
enum COUNTER_VALUE
{
	GOLD_DROP,
	MONSTER_KILL,
	SKILL_GAIN,
	MAX_COUNTER_VALUE,
};

class HEAT_MAP
{
public:
	HEAT_MAP( int Resolution );
	virtual ~HEAT_MAP();

public:
	void AddValue    ( COUNTER_VALUE Value, float value, int x, int y );
	float GetValue   ( COUNTER_VALUE Value, int startx, int endx, int starty, int endy );

	void Serialize   ( void* Buffer, size_t size );
	void Deserialize ( void* Buffer, size_t size );

	void DrawBitmap  ( HDC hdc, COUNTER_VALUE Value, int startx, int starty, int endx, int endy );

private:
	// A single value within a node
	struct COUNTER {
		COUNTER_VALUE Value;
		float v;
		COUNTER* Next;
	};

	// A single location, represented by an x,y coordinate.
	// Has many counters.
	struct NODE {
		int x;
		int y;
		COUNTER* Counters;
	};

	// Helper Functions
	NODE* FindNode         ( int x, int y );
	NODE* AllocateNode     ( int x, int y );
	void  AddValueToNode   ( NODE* Node, COUNTER_VALUE Value, float v );
	u32   CoordinateToZHash( const int _x, const int _y );
	float GetValueFromNode ( NODE* Node, COUNTER_VALUE Value );

	// Data
	int       Resolution;
	POOL_HEAP CounterHeap;
	POOL_HEAP NodeHeap;
	HASH_MAP  NodeMap;

	// Data Accessors
	COUNTER* GetFreeCounter( void );
	NODE*    GetFreeNode   ( void );

	// Header structure for serialization
	struct HEADER {
		bool  Serialized;
		int   NumNodes;
		int   NumCounters;
		NODE* Nodes;
		COUNTER* Counters;
	};
};

#endif