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
	NodeMap(100 * 1024)  // assume hundreds of thousands of nodes
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
HEAT_MAP::NODE* HEAT_MAP::FindNode( int x, int y )
{
	return (NODE*)NodeMap.Find(CoordinateToZHash(x/Resolution,y/Resolution));
}
//******************************************************************************
HEAT_MAP::NODE* HEAT_MAP::AllocateNode( int x, int y )
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
HEAT_MAP::COUNTER* HEAT_MAP::GetFreeCounter( void )
{
	return (COUNTER*)CounterHeap.GetBlock();
}
//******************************************************************************
HEAT_MAP::NODE* HEAT_MAP::GetFreeNode( void )
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
void HEAT_MAP::DrawBitmap( HDC hdc, COUNTER_VALUE Value, int startx, int starty, int endx, int endy )
{
	// Create some windows objects
	HBITMAP Bitmap = CreateCompatibleBitmap(hdc, endx - startx, endy - starty);
	SelectObject(hdc, Bitmap);
	
	// Fill screen with black
	RECT Rect;
	Rect.left = startx;
	Rect.top = starty;
	Rect.bottom = endy;
	Rect.right = endx;
	FillRect(hdc, &Rect, CreateSolidBrush(RGB(0,0,0)));

	// Write out the values we have
	uint Hash;
	NODE* Node;
	COUNTER* Counter;
	Node = (NODE*)NodeMap.GetFirst(&Hash);
	while( Node != null ) {
		Counter = Node->Counters;
		while(Counter != null) {
			if(Counter->Value == Value) {
				int r = clamp((int)Counter->v, 0, 10);
				r = (int)LinearInterpolate(r, 0, 10, 0, 255);
				SetPixel(hdc, Node->x, Node->y, RGB(r,0,0));
			}
			Counter = Counter->Next;
		}
		Node = (NODE*)NodeMap.GetNext(Hash, Node, &Hash);
	}
}