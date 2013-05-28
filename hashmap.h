//*****************************************************************************
// Hash Map
// --------
//   This is a simple hash map structure.
//
//   If multiple items hash to the same element, they will be appended to a
//   linked list in that bucket.
//
// @author Doug Frazer
// May 2013
//*****************************************************************************

#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "common_includes.h"

#include "pool_heap.h"

class HASH_MAP
{
public:
	HASH_MAP(uint NumBuckets);
	~HASH_MAP();

public:
	void  Init();
	void  Deinit();
	bool  Insert(u32 Hash, void* val);
	bool  Remove(u32 Hash);
	void* GetValue(u32 Hash);
	void* Find(u32 Hash) {return GetValue(Hash);}
	void* GetFirst(u32* Hash);
	void* GetNext(u32 Hash, void* Data, u32* OutHash);

private:
	struct BUCKET {
		u32     hash;
		void*   val;
		BUCKET* next;
	};

	BUCKET**  Buckets;
	uint NumBuckets;

	POOL_HEAP BucketHeap;

	uint    Size;

private:
	BUCKET** FindInternal(uint Hash, BUCKET** Prev = null);
	bool InsertInternal(uint Hash, void* key);
	bool InsertInternal(BUCKET* Bucket);
	bool Resize(uint NewNumBuckets);
	BUCKET* GetNewBucket();
	void FreeBucket(BUCKET* Bucket);
};

#endif