//******************************************************************************
// Pool Heap
// ---------
//
//   This pool heap allocates blocks of a fixed size, configured at instantiation.
//
//   It will dynamically resize itself by allocating a segment double the size of
//   the previous segment, effectively tripling the capacity, because it does not
//   free the previous segment.  This is done as an effort to prevent fragmentation.
//
//   We will store in our free memory a linked list to the next free block.  This is
//   done as an effort to reduce allocation overhead (to zero).
//
// @author Doug Frazer
// May 2013
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
	void Grow();

	struct BLOCK {
		BLOCK* Next;
	};

	struct SEGMENT {
		void*  Memory;
		BLOCK* FreeBlock;
	};
	static const uint MIN_BLOCKS_PER_NODE_LOG_2 = 6; // 2^6 blocks minimum = 64 blocks
	SEGMENT Segments[30 - MIN_BLOCKS_PER_NODE_LOG_2]; // enough for practically all addressable memory space
	
	int   LastSegment;
	int   PrevFreeSegment;
	int	  BlockSize;
};

#endif