//******************************************************************************
// Heat Map
// --------
//
// @author Doug Frazer
// May 2013
//******************************************************************************

#include "heatmap.h"


//******************************************************************************
// Constructor/Destructor
//******************************************************************************
HEAT_MAP::HEAT_MAP( int _Resolution ) :
	Resolution(_Resolution),
	CounterHeap(sizeof(COUNTER)),
	NodeHeap(sizeof(NODE)),
	NodeMap(1 * 1024 * 1024)  // assume millions of nodes
{
}
//******************************************************************************
HEAT_MAP::~HEAT_MAP()
{
}
//******************************************************************************


//******************************************************************************
// Public Interface
//******************************************************************************
void HEAT_MAP::AddValue( COUNTER_VALUE Value, float value, int x, int y )
{
	NODE* Node = FindNode(x,y);
	if(Node == null) {
		Node = AllocateNode(x,y);
	}
	assert(Node != null);

	AddValueToNode(Node, Value, value);
}
//******************************************************************************
float HEAT_MAP::GetValue( COUNTER_VALUE Value, int startx, int endx, int starty, int endy )
{
	float ret = 0.0;

	for(int x = startx/Resolution; x <= endx; x+= Resolution) {
		for(int y = starty/Resolution; y <= endy; y+= Resolution ) {
			NODE* Node = FindNode(x,y);
			if(Node != null) {
				ret += GetValueFromNode( Node, Value );
			}
		}
	}
	return ret;
}
//******************************************************************************
void HEAT_MAP::Serialize( void* Buffer, size_t size )
{
	int NumNodes;
	int NumCounters;
	u32 Hash;
	NODE* Node;
	COUNTER* Counter;

	// Scan once to figure out how much data we have and make sure we have enough space
	NumNodes = 0;
	NumCounters = 0;
	Node = (NODE*)NodeMap.GetFirst(&Hash);
	while( Node != null ) {
		NumNodes++;
		Counter = Node->Counters;
		while(Counter != null) {
			NumCounters++;
			Counter = Counter->Next;
		}
		Node = (NODE*)NodeMap.GetNext(Hash, Node, &Hash);
	}
	assert(NumNodes * sizeof(NODE) + NumCounters * sizeof(COUNTER) < size);

	// Setup the header pointers
	HEADER* Header = (HEADER*)Buffer;
	Header->NumNodes = NumNodes;
	Header->NumCounters = NumCounters;
	Header->Nodes = (NODE*)(Header + 1);
	Header->Counters = (COUNTER*)(&Header->Nodes[NumNodes]);

	// Write out the data
	NumNodes = 0;
	NumCounters = 0;
	Node = (NODE*)NodeMap.GetFirst(&Hash);
	while( Node != null ) {
		Header->Nodes[NumNodes] = *Node;
		Counter = Node->Counters;
		while(Counter != null) {
			COUNTER* RealNext = null;
			Header->Counters[NumCounters] = *Counter;
			if(Counter->Next != null) {
				RealNext = Counter->Next;
				Counter->Next = &Header->Counters[NumCounters + 1];
			}
			NumCounters++;
			Counter = RealNext != null ? RealNext : Counter->Next;
		}
		NumNodes++;
		Node = (NODE*)NodeMap.GetNext(Hash, Node, &Hash);
	}

	// Make all pointers relative
	for(int i = 0; i < Header->NumNodes; i++) {
		pointer_make_relative((void**)&Header->Nodes[i].Counters);
	}

	for(int i = 0; i < Header->NumCounters; i++) {
		Counter = &Header->Counters[i];
		while(Counter->Next != null) {
			COUNTER* Prev = Counter;
			Counter = Counter->Next;
			pointer_make_relative((void**)&Prev);
		}
	}
	pointer_make_relative((void**)&Header->Nodes);
	pointer_make_relative((void**)&Header->Counters);
	Header->Serialized = true;
}
//******************************************************************************
void HEAT_MAP::Deserialize( void* Buffer, size_t size )
{
	COUNTER* Counter;

	// Assume buffer starts with a header, make sure it appears copasetic
	HEADER* Header = (HEADER*)Buffer;
	assert(Header->NumCounters * sizeof(COUNTER) + Header->NumNodes * sizeof(NODE) < size);

	if(Header->Serialized) {
		// Make all pointers absolute
		pointer_make_absolute((void**)&Header->Nodes);
		pointer_make_absolute((void**)&Header->Counters);
		for(int i = 0; i < Header->NumNodes; i++) {
			pointer_make_absolute((void**)&Header->Nodes[i].Counters);
		}
		for(int i = 0; i < Header->NumCounters; i++) {
			Counter = &Header->Counters[i];
			while(Counter->Next != null) {
				COUNTER* Prev = Counter;
				Counter = Counter->Next;
				pointer_make_absolute((void**)&Prev);
			}
		}
		Header->Serialized = false;
	}

	// Add all the data to our current heat map
	for(int i = 0; i < Header->NumNodes; i++) {
		NODE* Node = &Header->Nodes[i];
		Counter = Node->Counters;
		while(Counter != null) {
			AddValue( Counter->Value, Counter->v, Node->x, Node->y );
			Counter = Counter->Next;
		}
	}
}
//******************************************************************************


//******************************************************************************
// Private Interface
//******************************************************************************
NODE* HEAT_MAP::FindNode( int x, int y )
{
	return (NODE*)NodeMap.Find(CoordinateToZHash(x/Resolution,y/Resolution));
}
//******************************************************************************
NODE* HEAT_MAP::AllocateNode( int x, int y )
{
	NODE* NewNode = GetFreeNode();
	NewNode->x = x;
	NewNode->y = y;
	NewNode->Counters = null;
	NodeMap.Insert(CoordinateToZHash(x/Resolution,y/Resolution), NewNode);
	return NewNode;
}
//******************************************************************************
void HEAT_MAP::AddValueToNode( NODE* Node, COUNTER_VALUE Value, float v )
{
	COUNTER* Curr = Node->Counters;
	while(Curr != null) {
		if(Curr->Value == Value) {
			Curr->v += v;
			return;
		}
		assert(Curr != Curr->Next);
		Curr = Curr->Next;
	}
	COUNTER* Counter = GetFreeCounter();
	assert(Counter != Node->Counters);
	Counter->v = v;
	Counter->Value = Value;
	Counter->Next = Node->Counters;
	Node->Counters = Counter;
}
//******************************************************************************
float HEAT_MAP::GetValueFromNode( NODE* Node, COUNTER_VALUE Value )
{
	COUNTER* Counters = Node->Counters;
	while(Counters != null) {
		if(Counters->Value == Value) {
			return Counters->v;
		}
		Counters = Counters->Next;
	}
	return 0.0;
}
//******************************************************************************
COUNTER* HEAT_MAP::GetFreeCounter( void )
{
	return (COUNTER*)CounterHeap.GetBlock();
}
//******************************************************************************
NODE* HEAT_MAP::GetFreeNode( void )
{
	return (NODE*)NodeHeap.GetBlock();
}
//******************************************************************************
// From http://graphics.stanford.edu/~seander/bithacks.html
//******************************************************************************
u32 HEAT_MAP::CoordinateToZHash( const int _x, const int _y )
{
	const int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
    const int S[] = {1, 2, 4, 8};

    int z;
	int x = _x;
	int y = _y;

    x = (x | (x << S[3])) & B[3];
    x = (x | (x << S[2])) & B[2];
    x = (x | (x << S[1])) & B[1];
    x = (x | (x << S[0])) & B[0];

    y = (y | (y << S[3])) & B[3];
    y = (y | (y << S[2])) & B[2];
    y = (y | (y << S[1])) & B[1];
    y = (y | (y << S[0])) & B[0];

    z = x | (y << 1);

    return z;
}
//******************************************************************************






//******************************************************************************
// DEBUG ONLY
//******************************************************************************
void HEAT_MAP::Draw( COUNTER_VALUE Value, int startx, int starty, int endx, int endy )
{
	static const char * ValueStrings[MAX_COUNTER_VALUE] = 
	{
	"GOLD_DROP",
	"MONSTER_KILL",
	"SKILL_GAIN",
	};
	char string[256];

	for(int x = startx/Resolution; x <= endx + Resolution; x+= Resolution) {
		OutputDebugString("*****");
	}
	OutputDebugString("\n");
	sprintf_s(string, 256, "- Heat map for %s\n", ValueStrings[Value]);
	OutputDebugString(string);
	for(int x = startx/Resolution; x <= endx + Resolution; x+= Resolution) {
		OutputDebugString("*****");
	}
	OutputDebugString("\n    ");
	for(int x = startx/Resolution; x <= endx; x+= Resolution) {
		sprintf_s(string, 256, " %3d ", x);
		OutputDebugString(string);
	}
	OutputDebugString("\n    ");
	for(int x = startx/Resolution; x <= endx + Resolution; x+= Resolution) {
		OutputDebugString("-----");
	}
	OutputDebugString("\n");

	for(int y = starty/Resolution; y <= endy; y += Resolution) {
		sprintf_s(string, 256, "%3d | ", y);
		OutputDebugString(string);
		for(int x = startx/Resolution; x <= endx; x += Resolution) {
			NODE* Node = FindNode(x,y);
			if(Node == null) {
				OutputDebugString("  .  ");
			} else {
				bool CounterFound = false;
				COUNTER* Counters = Node->Counters;
				while(Counters != null) {
					if(Counters->Value == Value) {
						sprintf_s(string, 256, " %3.0f ", Counters->v);
						OutputDebugString(string);
						CounterFound = true;
						break;
					}
					assert(Counters != Counters->Next);
					Counters = Counters->Next;
				}
				if(!CounterFound) OutputDebugString(" . ");
			}
		}
		sprintf_s(string, 256, "\n", y);
		OutputDebugString(string);
	}
	/*
	int TotalNodes = 0;
	u32 Hash;
	NODE* Node = (NODE*)NodeMap.GetFirst(&Hash);
	while( Node != null ) {
		sprintf_s(string, 256, "Node found: %d %d\n", Node->x, Node->y);
		OutputDebugString(string);
		Node = (NODE*)NodeMap.GetNext(Hash, Node, &Hash);
		TotalNodes++;
	}
	sprintf_s(string, 256, "Total Nodes: %d\n", TotalNodes);
	OutputDebugString(string);
	*/
}
//******************************************************************************
