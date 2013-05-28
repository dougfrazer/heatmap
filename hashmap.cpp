//*****************************************************************************
// Hash Map
// --------
//   See HashMap.h
//
// (c) March 2013
//
// @author Doug Frazer
//*****************************************************************************

#include "HashMap.h"
#include "string.h"
#include "stdlib.h"


//*****************************************************************************
// Constructor/Destructor
//*****************************************************************************
HASH_MAP::HASH_MAP(uint _NumBuckets) :
	BucketHeap(sizeof(BUCKET)),
	NumBuckets(_NumBuckets)
{
	Size = 0;
}
//*****************************************************************************
HASH_MAP::~HASH_MAP()
{	
}
//*****************************************************************************


//*****************************************************************************
// Public Interface
//*****************************************************************************
void HASH_MAP::Init()
{
	int BufferSize = alignup( NumBuckets*sizeof(BUCKET*), 8 );
    Buckets = (BUCKET**)Malloc( BufferSize, __FILE__, __LINE__ );
    if(Buckets == null) return;

    memset(Buckets, '\0', BufferSize);
}
//*****************************************************************************
void HASH_MAP::Deinit()
{
    BUCKET* Bucket;
    BUCKET* NextBucket;

    if(Buckets == null) return;
    for(uint i = 0; i < NumBuckets; i++) {
        Bucket = Buckets[i];
        while(Bucket != null) {
            NextBucket = Bucket->next;
            FreeBucket(Bucket);
            Bucket = NextBucket;
        }
    }
    Free(Buckets, __FILE__, __LINE__);
}
//*****************************************************************************
bool HASH_MAP::Insert(u32 Hash, void* val)
{
	return InsertInternal(Hash, val);
}
//*****************************************************************************
bool HASH_MAP::Remove(u32 Hash)
{
    BUCKET** Bucket;
    BUCKET*  Removed;

    Bucket = FindInternal(Hash);
    if(Bucket == null) {
        error("Out of memory");
        return false;
    }
    if(*Bucket == null) {
        error("Tried to remove a value that doesn't exist");
        return false;
    }

    Removed = *Bucket;
    *Bucket = Removed->next;
    FreeBucket(Removed);
    Size--;
    return true;
}
//*****************************************************************************
void* HASH_MAP::GetValue(u32 Hash)
{
    BUCKET** Bucket;

    Bucket = FindInternal(Hash);

    return Bucket == null ? null : *Bucket == null ? null : (*Bucket)->val;
}
//*****************************************************************************


//*****************************************************************************
// Private Interface
//*****************************************************************************
HASH_MAP::BUCKET** HASH_MAP::FindInternal(u32 Hash, BUCKET** Prev)
{
    uint Index;
    BUCKET**     Iter;

    if(Buckets == null) return null;

    Index = Hash % NumBuckets;
    Iter = &Buckets[Index];
    if(Prev != null) *Prev = null;

    while(*Iter != null) {
        if((*Iter)->hash == Hash) return Iter;
        if(Prev != null) *Prev = *Iter;
        Iter = &(*Iter)->next;
    }
    return Iter;
}
//*****************************************************************************
bool HASH_MAP::InsertInternal(u32 Hash, void* val)
{
    BUCKET*  Prev;
    BUCKET** Bucket;

    Bucket = FindInternal(Hash, &Prev);
    if(Bucket == null) {
        error("Out of memory");
        return false;
    }
    if(*Bucket != null) {
        error("Key already exists in map (or another key hashed to same value)");
        return false;
    }
    *Bucket = (BUCKET*)GetNewBucket();
    if(*Bucket == null) {
        return false;
    }
    (*Bucket)->hash = Hash;
    (*Bucket)->val = val;
    (*Bucket)->next = null;
    if(Prev != null) Prev->next = *Bucket;
    Size++;

	if(Size/(float)NumBuckets > 0.75) {
		if(!Resize(NumBuckets * 2)) {
			return false;
		}
	}

    return true;
}
//*****************************************************************************
bool HASH_MAP::InsertInternal(BUCKET* Bucket)
{
	BUCKET*  Prev;
    BUCKET** BucketLoc;

    BucketLoc = FindInternal(Bucket->hash, &Prev);
    if(BucketLoc == null) {
        error("Out of memory");
        return false;
    }
    if(*BucketLoc != null) {
        error("Key already exists in map (or another key hashed to same value)");
        return false;
    }
    *BucketLoc = Bucket;
	(*BucketLoc)->next = null;
    if(Prev != null) Prev->next = *BucketLoc;
    Size++;

	if(Size/(float)NumBuckets > 0.75) {
		if(!Resize(NumBuckets * 2)) {
			return false;
		}
	}

    return true;
}
//*****************************************************************************
bool HASH_MAP::Resize(uint NewNumBuckets)
{
    BUCKET** NewBuckets;
    BUCKET** OldBuckets;
    BUCKET*  Bucket;
    BUCKET*  NextBucket;
    uint     OldNumBuckets;

	int BufferSize = alignup(NewNumBuckets*sizeof(BUCKET*),8);
    NewBuckets = (BUCKET**)Malloc(BufferSize, __FILE__, __LINE__);
    if(NewBuckets == null) {
        error("Failed to alloc new space for resizing");
        return false;
    }
    memset(NewBuckets, '\0', BufferSize);

    OldBuckets = Buckets;
    OldNumBuckets = NumBuckets;

    Buckets = NewBuckets;
    NumBuckets = NewNumBuckets;
    Size = 0;
    for(uint i = 0; i < OldNumBuckets; i++) {
        Bucket = OldBuckets[i];
        while(Bucket != null) {
            NextBucket = Bucket->next;
            if(!InsertInternal(Bucket)) {
                error("Failed to insert bucket");
                return false;
            }
            Bucket = NextBucket;
        }
    }
    Free(OldBuckets, __FILE__, __LINE__);
    return true;
}
//*****************************************************************************
HASH_MAP::BUCKET* HASH_MAP::GetNewBucket()
{
	return (BUCKET*)BucketHeap.GetBlock();
}
//*****************************************************************************
void HASH_MAP::FreeBucket(BUCKET* Bucket)
{
	BucketHeap.FreeBlock(Bucket);
}
//*****************************************************************************
void* HASH_MAP::GetFirst(u32* Hash)
{
	for(uint i = 0; i < NumBuckets; i++) {
		if(Buckets[i] != null) {
			*Hash = Buckets[i]->hash;
			return Buckets[i]->val;
		}
	}
	return null;
}
//*****************************************************************************
void* HASH_MAP::GetNext(u32 Hash, void* Data, u32* OutHash)
{
	BUCKET* Bucket = Buckets[Hash % NumBuckets];
	bool BucketFound = false;

	while(Bucket != null) {
		if(Bucket->val == Data) {
			BucketFound = true;
			if(Bucket->next != null) {
				*OutHash = Bucket->next->hash;
				return Bucket->next->val;
			}
		}
		Bucket = Bucket->next;
	}
	assert(BucketFound);

	for(uint i = Hash % NumBuckets + 1; i < NumBuckets; i++) {
		if(Buckets[i] != null) {
			*OutHash = Buckets[i]->hash;
			return Buckets[i]->val;
		}
	}
	return null;
}
//*****************************************************************************