
#ifndef __HPHP_MIN_MAX_MACROS__
#define __HPHP_MIN_MAX_MACROS__

#ifdef MIN
#undef MIN
#endif
#define MIN(a, b)      ((a)>(b)?(b):(a))

#ifdef MAX
#undef MAX
#endif
#define MAX(a, b)      ((a)>(b)?(a):(b))

#endif // __HPHP_MIN_MAX_MACROS__

