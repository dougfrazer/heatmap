
#include "pool_heap.h"
#include "string.h"

//******************************************************************************
POOL_HEAP::POOL_HEAP(int _BlockSize) :
	BlockSize(_BlockSize),
	FreeSegment(0)
{
	assert(BlockSize > 0);
	memset(BlockSegments, '\0', sizeof(BlockSegments));
	Grow();
}
//******************************************************************************
POOL_HEAP::~POOL_HEAP()
{
}
//******************************************************************************
void* POOL_HEAP::GetBlock()
{
	for(int Segment = 0; Segment < FreeSegment; Segment++) {
		int NumBlocks = 0x1 << (Segment + MIN_BLOCKS_PER_NODE_LOG_2);
		for(int i = 0; i < NumBlocks / 8; i++) {
			if( (BlockSegments[Segment].BlockUsage[i] & (0x1 << 0)) == 0 ) { BlockSegments[Segment].BlockUsage[i] |= 0x1 << 0; return (void*)( (u8*)BlockSegments[Segment].Memory + BlockSize*(i*8 + 0) ); }
			if( (BlockSegments[Segment].BlockUsage[i] & (0x1 << 1)) == 0 ) { BlockSegments[Segment].BlockUsage[i] |= 0x1 << 1; return (void*)( (u8*)BlockSegments[Segment].Memory + BlockSize*(i*8 + 1) ); }
			if( (BlockSegments[Segment].BlockUsage[i] & (0x1 << 2)) == 0 ) { BlockSegments[Segment].BlockUsage[i] |= 0x1 << 2; return (void*)( (u8*)BlockSegments[Segment].Memory + BlockSize*(i*8 + 2) ); }
			if( (BlockSegments[Segment].BlockUsage[i] & (0x1 << 3)) == 0 ) { BlockSegments[Segment].BlockUsage[i] |= 0x1 << 3; return (void*)( (u8*)BlockSegments[Segment].Memory + BlockSize*(i*8 + 3) ); }
			if( (BlockSegments[Segment].BlockUsage[i] & (0x1 << 4)) == 0 ) { BlockSegments[Segment].BlockUsage[i] |= 0x1 << 4; return (void*)( (u8*)BlockSegments[Segment].Memory + BlockSize*(i*8 + 4) ); }
			if( (BlockSegments[Segment].BlockUsage[i] & (0x1 << 5)) == 0 ) { BlockSegments[Segment].BlockUsage[i] |= 0x1 << 5; return (void*)( (u8*)BlockSegments[Segment].Memory + BlockSize*(i*8 + 5) ); }
			if( (BlockSegments[Segment].BlockUsage[i] & (0x1 << 6)) == 0 ) { BlockSegments[Segment].BlockUsage[i] |= 0x1 << 6; return (void*)( (u8*)BlockSegments[Segment].Memory + BlockSize*(i*8 + 6) ); }
			if( (BlockSegments[Segment].BlockUsage[i] & (0x1 << 7)) == 0 ) { BlockSegments[Segment].BlockUsage[i] |= 0x1 << 7; return (void*)( (u8*)BlockSegments[Segment].Memory + BlockSize*(i*8 + 7) ); }
		}
	}
	Grow();
	return GetBlock();
}
//******************************************************************************
void POOL_HEAP::FreeBlock(void* Address)
{
	// Find the segment that it belongs to
	int Segment = 0;
	for( ; Segment <= FreeSegment; Segment++) {
		if((intptr_t)Address - (intptr_t)BlockSegments[Segment].Memory < BlockSize * (0x1 << (Segment + MIN_BLOCKS_PER_NODE_LOG_2)) &&
			(intptr_t)Address >= (intptr_t)BlockSegments[Segment].Memory ) {
			break;
		}
	}
	assert(Segment <= FreeSegment);

	// Relative difference between Memory and Address is the index into the BlockUsage array
	int BlockUsageAddr = ((intptr_t)Address - (intptr_t)BlockSegments[Segment].Memory);
	assert(BlockUsageAddr % BlockSize == 0);
	BlockUsageAddr /= BlockSize;
	int NumBlocks = 0x1 << (Segment + MIN_BLOCKS_PER_NODE_LOG_2);
	assert(BlockUsageAddr < NumBlocks);

	// Unset the "is in use" flag, clear the memory
	BlockSegments[Segment].BlockUsage[BlockUsageAddr/8] &= ~( 0x1 << BlockUsageAddr % 8 );
	memset(Address, '\0', BlockSize);
}
//******************************************************************************

void POOL_HEAP::Grow()
{
	assert(FreeSegment >= 0 && FreeSegment < countof(BlockSegments));
	int BufferSize = 0;
	int NumBlocks = 0x1 << (FreeSegment + MIN_BLOCKS_PER_NODE_LOG_2);
	BufferSize += NumBlocks * BlockSize;
	BufferSize += NumBlocks / 8;
	assert(BufferSize > 0);
	void* Buffer = Malloc(BufferSize, __FILE__, __LINE__);
	assert(Buffer != null);
	memset(Buffer, '\0', BufferSize);

	BlockSegments[FreeSegment].BlockUsage = (u8*)Buffer;
	BlockSegments[FreeSegment].Memory = (void*)&(BlockSegments[FreeSegment].BlockUsage[NumBlocks/8]);
	FreeSegment++;
}
//******************************************************************************
