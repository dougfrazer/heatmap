//******************************************************************************
// Pool Heap
// ---------
//
//   This pool heap allocates blocks of a fixed size, configured at instantiation.
//
//   It will retain a single bit for each block marking whether or not that
//   block is in use.  Therefore, the allocation overhead is NumBlocks / 8.
//
//   It will dynamically resize itself by allocating a segment double the size of
//   the previous segment, effectively tripling the capacity, because it does not
//   free the previous segment.  This is done as an effort to prevent fragmentation.
//   
//   The maximum number of segments needed to represent the addressible space is
//   64, because we are doubling everytime, that should be 2^64, assuming your first
//   segment is of size 1.  Since we define a start size of 64 bytes (2^6) we only 
//   need to store data for 58 possible segments.
//
//   @author Doug Frazer
//   May 2013
//******************************************************************************

#ifndef __POOL_HEAP_H__
#define __POOL_HEAP_H__

#include "common_includes.h"

class POOL_HEAP
{
public:
	POOL_HEAP(int BlockSize);
	virtual ~POOL_HEAP();

public:
	void* GetBlock();
	void  FreeBlock(void* Block);

private:
	void Grow(bool IncreaseSegment = false);

	struct SEGMENT {
		void* Memory;
		u8*   BlockUsage;
	};
	static const uint MIN_BLOCKS_PER_NODE_LOG_2 = 6; // 2^6 blocks minimum = 64 blocks
	SEGMENT BlockSegments[64 - MIN_BLOCKS_PER_NODE_LOG_2]; // enough for all addressable memory space
	int   FreeSegment;

	int	  BlockSize;

	int   PrevFreeBlock;
	int   PrevFreeSegment;
	int   PrevFreeIndex;
};

#endif