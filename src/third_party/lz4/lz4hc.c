/*
   LZ4 HC - High Compression Mode of LZ4
   Copyright (C) 2011-2012, Yann Collet.
   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - LZ4 homepage : http://fastcompression.blogspot.com/p/lz4.html
   - LZ4 source repository : http://code.google.com/p/lz4/
*/


//**************************************
// CPU Feature Detection
//**************************************
// 32 or 64 bits ?
#if (defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(__ppc64__) || defined(_WIN64) || defined(__LP64__) || defined(_LP64) )   // Detects 64 bits mode
#define LZ4_ARCH64 1
#else
#define LZ4_ARCH64 0
#endif

// Little Endian or Big Endian ? 
#if (defined(__BIG_ENDIAN__) || defined(__BIG_ENDIAN) || defined(_BIG_ENDIAN) || defined(_ARCH_PPC) || defined(__PPC__) || defined(__PPC) || defined(PPC) || defined(__powerpc__) || defined(__powerpc) || defined(powerpc) || ((defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))) )
#define LZ4_BIG_ENDIAN 1
#else
// Little Endian assumed. PDP Endian and other very rare endian format are unsupported.
#endif

// Unaligned memory access is automatically enabled for "common" CPU, such as x86.
// For others CPU, the compiler will be more cautious, and insert extra code to ensure aligned access is respected
// If you know your target CPU supports unaligned memory access, you may want to force this option manually to improve performance
#if defined(__ARM_FEATURE_UNALIGNED)
#define LZ4_FORCE_UNALIGNED_ACCESS 1
#endif


//**************************************
// Compiler Options
//**************************************
#if __STDC_VERSION__ >= 199901L    // C99
  /* "restrict" is a known keyword */
#else
#define restrict  // Disable restrict
#endif

#ifdef _MSC_VER
#define inline __forceinline    // Visual is not C99, but supports some kind of inline
#include <intrin.h>             // For Visual 2005
#  if LZ4_ARCH64	// 64-bit
#    pragma intrinsic(_BitScanForward64) // For Visual 2005
#    pragma intrinsic(_BitScanReverse64) // For Visual 2005
#  else
#    pragma intrinsic(_BitScanForward)   // For Visual 2005
#    pragma intrinsic(_BitScanReverse)   // For Visual 2005
#  endif
#endif

#ifdef _MSC_VER  // Visual Studio
#define lz4_bswap16(x) _byteswap_ushort(x)
#else
#define lz4_bswap16(x)  ((unsigned short int) ((((x) >> 8) & 0xffu) | (((x) & 0xffu) << 8)))
#endif


//**************************************
// Includes
//**************************************
#include <stdlib.h>   // calloc, free
#include <string.h>   // memset, memcpy
#include "lz4hc.h"

#define ALLOCATOR(s) calloc(1,s)
#define FREEMEM free
#define MEM_INIT memset


//**************************************
// Basic Types
//**************************************
#if defined(_MSC_VER)    // Visual Studio does not support 'stdint' natively
#define BYTE	unsigned __int8
#define U16		unsigned __int16
#define U32		unsigned __int32
#define S32		__int32
#define U64		unsigned __int64
#else
#include <stdint.h>
#define BYTE	uint8_t
#define U16		uint16_t
#define U32		uint32_t
#define S32		int32_t
#define U64		uint64_t
#endif

#ifndef LZ4_FORCE_UNALIGNED_ACCESS
#pragma pack(push, 1) 
#endif

typedef struct _U16_S { U16 v; } U16_S;
typedef struct _U32_S { U32 v; } U32_S;
typedef struct _U64_S { U64 v; } U64_S;

#ifndef LZ4_FORCE_UNALIGNED_ACCESS
#pragma pack(pop) 
#endif

#define A64(x) (((U64_S *)(x))->v)
#define A32(x) (((U32_S *)(x))->v)
#define A16(x) (((U16_S *)(x))->v)


//**************************************
// Constants
//**************************************
#define MINMATCH 4

#define DICTIONARY_LOGSIZE 16
#define MAXD (1<<DICTIONARY_LOGSIZE)
#define MAXD_MASK ((U32)(MAXD - 1))
#define MAX_DISTANCE (MAXD - 1)

#define HASH_LOG (DICTIONARY_LOGSIZE-1)
#define HASHTABLESIZE (1 << HASH_LOG)
#define HASH_MASK (HASHTABLESIZE - 1)

#define MAX_NB_ATTEMPTS 256

#define ML_BITS  4
#define ML_MASK  (size_t)((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)

#define COPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (COPYLENGTH+MINMATCH)
#define MINLENGTH (MFLIMIT+1)
#define OPTIMAL_ML (int)((ML_MASK-1)+MINMATCH)


//**************************************
// Architecture-specific macros
//**************************************
#if LZ4_ARCH64	// 64-bit
#define STEPSIZE 8
#define LZ4_COPYSTEP(s,d)		A64(d) = A64(s); d+=8; s+=8;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d)
#define UARCH U64
#define AARCH A64
#define HTYPE					U32
#define INITBASE(b,s)			const BYTE* const b = s
#else		// 32-bit
#define STEPSIZE 4
#define LZ4_COPYSTEP(s,d)		A32(d) = A32(s); d+=4; s+=4;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d); LZ4_COPYSTEP(s,d);
#define UARCH U32
#define AARCH A32
#define HTYPE					const BYTE*
#define INITBASE(b,s)		    const int b = 0
#endif

#if defined(LZ4_BIG_ENDIAN)
#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { U16 v = A16(p); v = lz4_bswap16(v); d = (s) - v; }
#define LZ4_WRITE_LITTLEENDIAN_16(p,i)  { U16 v = (U16)(i); v = lz4_bswap16(v); A16(p) = v; p+=2; }
#else		// Little Endian
#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { d = (s) - A16(p); }
#define LZ4_WRITE_LITTLEENDIAN_16(p,v)  { A16(p) = v; p+=2; }
#endif


//************************************************************
// Local Types
//************************************************************
typedef struct 
{
	const BYTE* base;
	HTYPE hashTable[HASHTABLESIZE];
	U16 chainTable[MAXD];
	const BYTE* nextToUpdate;
} LZ4HC_Data_Structure;


//**************************************
// Macros
//**************************************
#define LZ4_WILDCOPY(s,d,e)		do { LZ4_COPYPACKET(s,d) } while (d<e);
#define LZ4_BLINDCOPY(s,d,l)	{ BYTE* e=d+l; LZ4_WILDCOPY(s,d,e); d=e; }
#define HASH_FUNCTION(i)	(((i) * 2654435761U) >> ((MINMATCH*8)-HASH_LOG))
#define HASH_VALUE(p)		HASH_FUNCTION(*(U32*)(p))
#define HASH_POINTER(p)		(HashTable[HASH_VALUE(p)] + base)
#define DELTANEXT(p)		chainTable[(size_t)(p) & MAXD_MASK] 
#define GETNEXT(p)			((p) - (size_t)DELTANEXT(p))
#define ADD_HASH(p)			{ size_t delta = (p) - HASH_POINTER(p); if (delta>MAX_DISTANCE) delta = MAX_DISTANCE; DELTANEXT(p) = (U16)delta; HashTable[HASH_VALUE(p)] = (p) - base; }


//**************************************
// Private functions
//**************************************
#if LZ4_ARCH64

inline static int LZ4_NbCommonBytes (register U64 val)
{
#if defined(LZ4_BIG_ENDIAN)
    #if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
    unsigned long r = 0;
    _BitScanReverse64( &r, val );
    return (int)(r>>3);
    #elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304) && !defined(LZ4_FORCE_SW_BITCOUNT)
    return (__builtin_clzll(val) >> 3); 
    #else
	int r;
	if (!(val>>32)) { r=4; } else { r=0; val>>=32; }
	if (!(val>>16)) { r+=2; val>>=8; } else { val>>=24; }
	r += (!val);
	return r;
    #endif
#else
    #if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
    unsigned long r = 0;
    _BitScanForward64( &r, val );
    return (int)(r>>3);
    #elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304) && !defined(LZ4_FORCE_SW_BITCOUNT)
    return (__builtin_ctzll(val) >> 3); 
    #else
	static const int DeBruijnBytePos[64] = { 0, 0, 0, 0, 0, 1, 1, 2, 0, 3, 1, 3, 1, 4, 2, 7, 0, 2, 3, 6, 1, 5, 3, 5, 1, 3, 4, 4, 2, 5, 6, 7, 7, 0, 1, 2, 3, 3, 4, 6, 2, 6, 5, 5, 3, 4, 5, 6, 7, 1, 2, 4, 6, 4, 4, 5, 7, 2, 6, 5, 7, 6, 7, 7 };
	return DeBruijnBytePos[((U64)((val & -val) * 0x0218A392CDABBD3F)) >> 58];
    #endif
#endif
}

#else

inline static int LZ4_NbCommonBytes (register U32 val)
{
#if defined(LZ4_BIG_ENDIAN)
    #if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
    unsigned long r = 0;
    _BitScanReverse( &r, val );
    return (int)(r>>3);
    #elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304) && !defined(LZ4_FORCE_SW_BITCOUNT)
    return (__builtin_clz(val) >> 3); 
    #else
	int r;
	if (!(val>>16)) { r=2; val>>=8; } else { r=0; val>>=24; }
	r += (!val);
	return r;
    #endif
#else
    #if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
    unsigned long r = 0;
    _BitScanForward( &r, val );
    return (int)(r>>3);
    #elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304) && !defined(LZ4_FORCE_SW_BITCOUNT)
    return (__builtin_ctz(val) >> 3); 
    #else
	static const int DeBruijnBytePos[32] = { 0, 0, 3, 0, 3, 1, 3, 0, 3, 2, 2, 1, 3, 2, 0, 1, 3, 3, 1, 2, 2, 2, 2, 0, 3, 1, 2, 0, 1, 0, 1, 1 };
	return DeBruijnBytePos[((U32)((val & -(S32)val) * 0x077CB531U)) >> 27];
    #endif
#endif
}

#endif


inline static int LZ4HC_Init (LZ4HC_Data_Structure* hc4, const BYTE* base)
{
	MEM_INIT((void*)hc4->hashTable, 0, sizeof(hc4->hashTable));
	MEM_INIT(hc4->chainTable, 0xFF, sizeof(hc4->chainTable));
	hc4->nextToUpdate = base + LZ4_ARCH64;
	hc4->base = base;
	return 1;
}


inline static void* LZ4HC_Create (const BYTE* base)
{
	void* hc4 = ALLOCATOR(sizeof(LZ4HC_Data_Structure));

	LZ4HC_Init (hc4, base);
	return hc4;
}


inline static int LZ4HC_Free (void** LZ4HC_Data)
{
	FREEMEM(*LZ4HC_Data);
	*LZ4HC_Data = NULL;
	return (1);
}


inline static void LZ4HC_Insert (LZ4HC_Data_Structure* hc4, const BYTE* ip)
{
	U16*   chainTable = hc4->chainTable;
	HTYPE* HashTable  = hc4->hashTable;
	INITBASE(base,hc4->base);

	while(hc4->nextToUpdate < ip)
	{
		ADD_HASH(hc4->nextToUpdate);
		hc4->nextToUpdate++;
	}
}


inline static int LZ4HC_InsertAndFindBestMatch (LZ4HC_Data_Structure* hc4, const BYTE* ip, const BYTE* const matchlimit, const BYTE** matchpos)
{
	U16* const chainTable = hc4->chainTable;
	HTYPE* const HashTable = hc4->hashTable;
	const BYTE* ref;
	INITBASE(base,hc4->base);
	int nbAttempts=MAX_NB_ATTEMPTS;
	int ml=0;

	// HC4 match finder
	LZ4HC_Insert(hc4, ip);
	ref = HASH_POINTER(ip);
	while ((ref > (ip-MAX_DISTANCE)) && (nbAttempts))
	{
		nbAttempts--;
		if (*(ref+ml) == *(ip+ml))
		if (*(U32*)ref == *(U32*)ip)
		{
			const BYTE* reft = ref+MINMATCH;
			const BYTE* ipt = ip+MINMATCH;

			while (ipt<matchlimit-(STEPSIZE-1))
			{
				UARCH diff = AARCH(reft) ^ AARCH(ipt);
				if (!diff) { ipt+=STEPSIZE; reft+=STEPSIZE; continue; }
				ipt += LZ4_NbCommonBytes(diff);
				goto _endCount;
			}
			if (LZ4_ARCH64) if ((ipt<(matchlimit-3)) && (A32(reft) == A32(ipt))) { ipt+=4; reft+=4; }
			if ((ipt<(matchlimit-1)) && (A16(reft) == A16(ipt))) { ipt+=2; reft+=2; }
			if ((ipt<matchlimit) && (*reft == *ipt)) ipt++;
_endCount:

			if (ipt-ip > ml) { ml = (int)(ipt-ip); *matchpos = ref; }
		}
		ref = GETNEXT(ref);
	}

	return ml;
}


inline static int LZ4HC_InsertAndGetWiderMatch (LZ4HC_Data_Structure* hc4, const BYTE* ip, const BYTE* startLimit, const BYTE* matchlimit, int longest, const BYTE** matchpos, const BYTE** startpos)
{
	U16* const  chainTable = hc4->chainTable;
	HTYPE* const HashTable = hc4->hashTable;
	INITBASE(base,hc4->base);
	const BYTE*  ref;
	int nbAttempts = MAX_NB_ATTEMPTS;
	int delta = (int)(ip-startLimit);

	// First Match
	LZ4HC_Insert(hc4, ip);
	ref = HASH_POINTER(ip);

	while ((ref > ip-MAX_DISTANCE) && (ref >= hc4->base) && (nbAttempts))
	{
		nbAttempts--;
		if (*(startLimit + longest) == *(ref - delta + longest))
		if (*(U32*)ref == *(U32*)ip)
		{
			const BYTE* reft = ref+MINMATCH;
			const BYTE* ipt = ip+MINMATCH;
			const BYTE* startt = ip;

			while (ipt<matchlimit-(STEPSIZE-1))
			{
				UARCH diff = AARCH(reft) ^ AARCH(ipt);
				if (!diff) { ipt+=STEPSIZE; reft+=STEPSIZE; continue; }
				ipt += LZ4_NbCommonBytes(diff);
				goto _endCount;
			}
			if (LZ4_ARCH64) if ((ipt<(matchlimit-3)) && (A32(reft) == A32(ipt))) { ipt+=4; reft+=4; }
			if ((ipt<(matchlimit-1)) && (A16(reft) == A16(ipt))) { ipt+=2; reft+=2; }
			if ((ipt<matchlimit) && (*reft == *ipt)) ipt++;
_endCount:

			reft = ref;
			while ((startt>startLimit) && (reft > hc4->base) && (startt[-1] == reft[-1])) {startt--; reft--;}

			if ((ipt-startt) > longest)
			{
				longest = (int)(ipt-startt);
				*matchpos = reft;
				*startpos = startt;
			}
		}
		ref = GETNEXT(ref);
	}

	return longest;
}


inline static int LZ4_encodeSequence(const BYTE** ip, BYTE** op, const BYTE** anchor, int ml, const BYTE* ref)
{
	int length, len; 
	BYTE* token;

	// Encode Literal length
	length = (int)(*ip - *anchor);
	token = (*op)++;
	if (length>=(int)RUN_MASK) { *token=(RUN_MASK<<ML_BITS); len = length-RUN_MASK; for(; len > 254 ; len-=255) *(*op)++ = 255;  *(*op)++ = (BYTE)len; } 
	else *token = (length<<ML_BITS);

	// Copy Literals
	LZ4_BLINDCOPY(*anchor, *op, length);

	// Encode Offset
	LZ4_WRITE_LITTLEENDIAN_16(*op,(U16)(*ip-ref));

	// Encode MatchLength
	len = (int)(ml-MINMATCH);
	if (len>=(int)ML_MASK) { *token+=ML_MASK; len-=ML_MASK; for(; len > 509 ; len-=510) { *(*op)++ = 255; *(*op)++ = 255; } if (len > 254) { len-=255; *(*op)++ = 255; } *(*op)++ = (BYTE)len; } 
	else *token += len;	

	// Prepare next loop
	*ip += ml;
	*anchor = *ip; 

	return 0;
}


//****************************
// Compression CODE
//****************************

int LZ4_compressHCCtx(LZ4HC_Data_Structure* ctx,
				 const char* source, 
				 char* dest,
				 int isize)
{	
	const BYTE* ip = (const BYTE*) source;
	const BYTE* anchor = ip;
	const BYTE* const iend = ip + isize;
	const BYTE* const mflimit = iend - MFLIMIT;
	const BYTE* const matchlimit = (iend - LASTLITERALS);

	BYTE* op = (BYTE*) dest;

	int	ml, ml2, ml3, ml0;
	const BYTE* ref=NULL;
	const BYTE* start2=NULL;
	const BYTE* ref2=NULL;
	const BYTE* start3=NULL;
	const BYTE* ref3=NULL;
	const BYTE* start0;
	const BYTE* ref0;

	ip++;

	// Main Loop
	while (ip < mflimit)
	{
		ml = LZ4HC_InsertAndFindBestMatch (ctx, ip, matchlimit, (&ref));
		if (!ml) { ip++; continue; }

		// saved, in case we would skip too much
		start0 = ip;
		ref0 = ref;
		ml0 = ml;

_Search2:
		if (ip+ml < mflimit)
			ml2 = LZ4HC_InsertAndGetWiderMatch(ctx, ip + ml - 2, ip + 1, matchlimit, ml, &ref2, &start2);
		else ml2=ml;

		if (ml2 == ml)  // No better match
		{
			LZ4_encodeSequence(&ip, &op, &anchor, ml, ref);
			continue;
		}

		if (start0 < ip)
		{
			if (start2 < ip + ml0)   // empirical
			{
				ip = start0;
				ref = ref0;
				ml = ml0;
			}
		}

		// Here, start0==ip
		if ((start2 - ip) < 3)   // First Match too small : removed
		{
			ml = ml2;
			ip = start2;
			ref =ref2;
			goto _Search2;
		}

_Search3:
		// Currently we have :
		// ml2 > ml1, and
		// ip1+3 <= ip2 (usually < ip1+ml1)
		if ((start2 - ip) < OPTIMAL_ML)
		{
			int correction;
			int new_ml = ml;
			if (new_ml > OPTIMAL_ML) new_ml = OPTIMAL_ML;
			if (ip+new_ml > start2 + ml2 - MINMATCH) new_ml = (int)(start2 - ip) + ml2 - MINMATCH;
			correction = new_ml - (int)(start2 - ip);
			if (correction > 0)
			{
				start2 += correction;
				ref2 += correction;
				ml2 -= correction;
			}
		}
		// Now, we have start2 = ip+new_ml, with new_ml=min(ml, OPTIMAL_ML=18)

		if (start2 + ml2 < mflimit)
			ml3 = LZ4HC_InsertAndGetWiderMatch(ctx, start2 + ml2 - 3, start2, matchlimit, ml2, &ref3, &start3);
		else ml3=ml2;

		if (ml3 == ml2) // No better match : 2 sequences to encode
		{
			// ip & ref are known; Now for ml
			if (start2 < ip+ml)
			{
				if ((start2 - ip) < OPTIMAL_ML)
				{
					int correction;
					if (ml > OPTIMAL_ML) ml = OPTIMAL_ML;
					if (ip+ml > start2 + ml2 - MINMATCH) ml = (int)(start2 - ip) + ml2 - MINMATCH;
					correction = ml - (int)(start2 - ip);
					if (correction > 0)
					{
						start2 += correction;
						ref2 += correction;
						ml2 -= correction;
					}
				}
				else
				{
					ml = (int)(start2 - ip);
				}
			}
			// Now, encode 2 sequences
			LZ4_encodeSequence(&ip, &op, &anchor, ml, ref);
			ip = start2;
			LZ4_encodeSequence(&ip, &op, &anchor, ml2, ref2);
			continue;
		}

		if (start3 < ip+ml+3) // Not enough space for match 2 : remove it
		{
			if (start3 >= (ip+ml)) // can write Seq1 immediately ==> Seq2 is removed, so Seq3 becomes Seq1
			{
				if (start2 < ip+ml)
				{
					int correction = (int)(ip+ml - start2);
					start2 += correction;
					ref2 += correction;
					ml2 -= correction;
					if (ml2 < MINMATCH)
					{
						start2 = start3;
						ref2 = ref3;
						ml2 = ml3;
					}
				}

				LZ4_encodeSequence(&ip, &op, &anchor, ml, ref);
				ip  = start3;
				ref = ref3;
				ml  = ml3;

				start0 = start2;
				ref0 = ref2;
				ml0 = ml2;
				goto _Search2;
			}

			start2 = start3;
			ref2 = ref3;
			ml2 = ml3;
			goto _Search3;
		}

		// OK, now we have 3 ascending matches; let's write at least the first one
		// ip & ref are known; Now for ml
		if (start2 < ip+ml)
		{
			if ((start2 - ip) < (int)ML_MASK)
			{
				int correction;
				if (ml > OPTIMAL_ML) ml = OPTIMAL_ML;
				if (ip + ml > start2 + ml2 - MINMATCH) ml = (int)(start2 - ip) + ml2 - MINMATCH;
				correction = ml - (int)(start2 - ip);
				if (correction > 0)
				{
					start2 += correction;
					ref2 += correction;
					ml2 -= correction;
				}
			}
			else
			{
				ml = (int)(start2 - ip);
			}
		}
		LZ4_encodeSequence(&ip, &op, &anchor, ml, ref);

		ip = start2;
		ref = ref2;
		ml = ml2;

		start2 = start3;
		ref2 = ref3;
		ml2 = ml3;

		goto _Search3;

	}

	// Encode Last Literals
	{
		int lastRun = (int)(iend - anchor);
		if (lastRun>=(int)RUN_MASK) { *op++=(RUN_MASK<<ML_BITS); lastRun-=RUN_MASK; for(; lastRun > 254 ; lastRun-=255) *op++ = 255; *op++ = (BYTE) lastRun; } 
		else *op++ = (lastRun<<ML_BITS);
		memcpy(op, anchor, iend - anchor);
		op += iend-anchor;
	} 

	// End
	return (int) (((char*)op)-dest);
}


int LZ4_compressHC(const char* source, 
				 char* dest,
				 int isize)
{
	void* ctx = LZ4HC_Create((const BYTE*)source);
	int result = LZ4_compressHCCtx(ctx, source, dest, isize);
	LZ4HC_Free (&ctx);

	return result;
}


