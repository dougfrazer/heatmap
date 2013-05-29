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
}
//******************************************************************************
POOL_HEAP::~POOL_HEAP()
{
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
