
// Fast data compression library
// Copyright (C) 2006-2009 Lasse Mikkel Reinhold
// lar@quicklz.com
//
// QuickLZ can be used for free under the GPL-1 or GPL-2 license (where anything
// released into public must be open source) or under a commercial license if such
// has been acquired (see http://www.quicklz.com/order.html). The commercial license
// does not cover derived or ported versions created by third parties under GPL.

// You can edit following user settings. Data must be decompressed with the same
// setting of QLZ_COMPRESSION_LEVEL and QLZ_STREAMING_BUFFER as it was compressed
// (see manual). If QLZ_STREAMING_BUFFER > 0, scratch buffers must be initially
// zeroed out (see manual). First #ifndef makes it possible to define settings from
// the outside like the compiler command line.

// BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION
// BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION
// BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION
// BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION
// BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION BETA VERSION

// 1.5.0 BETA 2

#ifndef QLZ_COMPRESSION_LEVEL
	#define QLZ_COMPRESSION_LEVEL 1
	//#define QLZ_COMPRESSION_LEVEL 2
	//#define QLZ_COMPRESSION_LEVEL 3

	#define QLZ_STREAMING_BUFFER 0
	//#define QLZ_STREAMING_BUFFER 100000
	//#define QLZ_STREAMING_BUFFER 1000000

	//#define QLZ_MEMORY_SAFE
#endif

#define QLZ_VERSION_MAJOR 1
#define QLZ_VERSION_MINOR 5
#define QLZ_VERSION_REVISION 0

// Verify compression level
#if QLZ_COMPRESSION_LEVEL != 1 && QLZ_COMPRESSION_LEVEL != 2 && QLZ_COMPRESSION_LEVEL != 3
#error QLZ_COMPRESSION_LEVEL must be 1, 2 or 3
#endif

typedef unsigned int ui32;
typedef unsigned short int ui16;

#ifdef QLZ_POINTERS
#undef QLZ_POINTERS
#undef QLZ_HASH_VALUES
#endif

// Decrease QLZ_POINTERS for level 3 to increase compression speed. Do not touch any other values!
#if QLZ_COMPRESSION_LEVEL == 1
#define QLZ_POINTERS 1
#define QLZ_HASH_VALUES 4096
#elif QLZ_COMPRESSION_LEVEL == 2
#define QLZ_POINTERS 4
#define QLZ_HASH_VALUES 2048
#elif QLZ_COMPRESSION_LEVEL == 3
#define QLZ_POINTERS 16
#define QLZ_HASH_VALUES 4096
#endif

// Detect if pointer size is 64-bit. It's not fatal if some 64-bit target is not detected because this is only for adding an optional 64-bit optimization.
#if defined _LP64 || defined __LP64__ || defined __64BIT__ || _ADDR64 || defined _WIN64 || defined __arch64__ || __WORDSIZE == 64 || (defined __sparc && defined __sparcv9) || defined __x86_64 || defined __amd64 || defined __x86_64__ || defined _M_X64 || defined _M_IA64 || defined __ia64 || defined __IA64__
	#define QLZ_PTR_64
#endif

// hash entry
typedef struct
{
#if QLZ_COMPRESSION_LEVEL == 1
	ui32 cache;
#if defined QLZ_PTR_64 && QLZ_STREAMING_BUFFER == 0
	unsigned int offset;
#else
	const unsigned char *offset;
#endif
#else
	const unsigned char *offset[QLZ_POINTERS];
#endif

} qlz_hash_compress;

typedef struct
{
#if QLZ_COMPRESSION_LEVEL == 1
	const unsigned char *offset;
#else
	const unsigned char *offset[QLZ_POINTERS];
#endif
} qlz_hash_decompress;


// states
typedef struct
{
	#if QLZ_STREAMING_BUFFER > 0
		unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
	#endif
	size_t stream_counter;
	qlz_hash_compress hash[QLZ_HASH_VALUES];
	unsigned char hash_counter[QLZ_HASH_VALUES];
} qlz_state_compress;


#if QLZ_COMPRESSION_LEVEL == 1 || QLZ_COMPRESSION_LEVEL == 2
	typedef struct
	{
#if QLZ_STREAMING_BUFFER > 0
		unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
#endif
		qlz_hash_decompress hash[QLZ_HASH_VALUES];
		unsigned char hash_counter[QLZ_HASH_VALUES];
		size_t stream_counter;
	} qlz_state_decompress;
#elif QLZ_COMPRESSION_LEVEL == 3
	typedef struct
	{
#if QLZ_STREAMING_BUFFER > 0
		unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
#endif
		qlz_hash_decompress hash[QLZ_HASH_VALUES];
		size_t stream_counter;
	} qlz_state_decompress;
#endif


// Public functions of QuickLZ
size_t qlz_size_decompressed(const char *source);
size_t qlz_size_compressed(const char *source);
size_t qlz_compress(const void *source, char *destination, size_t size, qlz_state_compress *state);
size_t qlz_decompress(const char *source, void *destination, qlz_state_decompress *state);
int qlz_get_setting(int setting);
