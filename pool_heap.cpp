//******************************************************************************
// Pool Heap
// ---------
//    See pool_heap.h
//
// May 2013
//******************************************************************************
#include "pool_heap.h"
#include "string.h"

//******************************************************************************
POOL_HEAP::POOL_HEAP(int _BlockSize) :
	BlockSize(_BlockSize),
	LastSegment(0),
	PrevFreeSegment(0)
{
	assert(BlockSize > sizeof(BLOCK)); // we reuse free space to store data, so make sure its at least big enough
	memset(Segments, '\0', sizeof(Segments));
	Grow();

	TestFree();
}
//******************************************************************************
POOL_HEAP::~POOL_HEAP()
{
	for(int Segment = 0; Segment <= LastSegment; Segment++) {
		Free(Segments[Segment].Memory, __FILE__, __LINE__);
	}
}
//******************************************************************************
void* POOL_HEAP::GetBlock()
{
	int Segment = PrevFreeSegment;
	do {
		if(Segments[Segment].FreeBlock == null) {
			Segment = Segment + 1 == LastSegment ? 0 : Segment + 1;
			continue;
		}
		BLOCK* PrevBlock = Segments[Segment].FreeBlock;
		Segments[Segment].FreeBlock = Segments[Segment].FreeBlock->Next;
		return PrevBlock;
	} while(Segment != PrevFreeSegment);
	
	Grow();
	return GetBlock();
}
//******************************************************************************
void POOL_HEAP::FreeBlock(void* Address)
{
	// Find the segment that it belongs to, this is a max of a 24 object search, usually much smaller
	int Segment = 0;
	for( ; Segment <= LastSegment; Segment++) {
		if((intptr_t)Address - (intptr_t)Segments[Segment].Memory < BlockSize * (0x1 << (Segment + MIN_BLOCKS_PER_NODE_LOG_2)) &&
			(intptr_t)Address >= (intptr_t)Segments[Segment].Memory ) {
			break;
		}
	}
	int NumBlocks = 0x1 << (Segment + MIN_BLOCKS_PER_NODE_LOG_2);
	intptr_t BlockAddr = ((intptr_t)Address - (intptr_t)Segments[Segment].Memory);
	assert(Segment <= LastSegment);
	assert(BlockAddr % BlockSize == 0);
	assert(BlockAddr/NumBlocks < NumBlocks);

	// Relative difference between Memory and Address is the index into the BlockUsage array
	BLOCK* PrevFreeBlock = Segments[Segment].FreeBlock;
	Segments[Segment].FreeBlock = (BLOCK*)Address;
	Segments[Segment].FreeBlock->Next = PrevFreeBlock;
}
//******************************************************************************

void POOL_HEAP::Grow()
{
	assert(LastSegment >= 0 && LastSegment < countof(Segments));
	int BufferSize = 0;
	int NumBlocks = 0x1 << (LastSegment + MIN_BLOCKS_PER_NODE_LOG_2);
	BufferSize += NumBlocks * BlockSize;
	assert(BufferSize > 0);
	void* Buffer = Malloc(BufferSize, __FILE__, __LINE__);
	assert(Buffer != null);

	Segments[LastSegment].Memory = Buffer;

	BLOCK* Block;
	BLOCK* NextBlock;
	for(int i = 0; i < NumBlocks - 1; i++) {
		Block =     (BLOCK*)( (u8*)Segments[LastSegment].Memory + BlockSize*(i) );
		NextBlock = (BLOCK*)( (u8*)Segments[LastSegment].Memory + BlockSize*(i + 1) );
		Block->Next = NextBlock;
	}
	BLOCK* LastBlock = (BLOCK*)( (u8*)Segments[LastSegment].Memory + BlockSize*(NumBlocks - 1) );
	LastBlock->Next = null;

	Segments[LastSegment].FreeBlock = (BLOCK*)Segments[LastSegment].Memory;
	LastSegment++;
}
//******************************************************************************

//******************************************************************************
// TEST CODE
//******************************************************************************
uint POOL_HEAP::GetNumFreeNodes()
{
	int counter = 0;
	for(int Segment = 0; Segment <= LastSegment; Segment++) {
		BLOCK* Curr = Segments[Segment].FreeBlock;
		while(Curr != null) {
			counter++;
			Curr = Curr->Next;
		}
	}
	return counter;
}
//******************************************************************************
void POOL_HEAP::TestFree()
{
	uint FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 64);
	BLOCK* Block1 = (BLOCK*)GetBlock();
	BLOCK* Block2 = (BLOCK*)GetBlock();
	BLOCK* Block3 = (BLOCK*)GetBlock();
	BLOCK* Block4 = (BLOCK*)GetBlock();
	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 60);
	FreeBlock(Block3);
	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 61);
	FreeBlock(Block1);
	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 62);
	FreeBlock(Block2);
	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 63);
	FreeBlock(Block4);
	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 64);

	BLOCK* Temp[128];
	for(int i = 0; i < 65; i++) {
		Temp[i] = (BLOCK*)GetBlock();
	}

	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 127);

	FreeBlock(Temp[13]);

	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 128);

	FreeBlock(Temp[64]);

	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 129);
	
	for(int i = 0; i < 65; i++) {
		if(i == 13 || i == 64) continue;
		FreeBlock(Temp[i]);
	}

	FreeBlocks = GetNumFreeNodes();
	assert(FreeBlocks == 128 + 64);
}
//******************************************************************************