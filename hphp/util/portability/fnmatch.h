// The actual implementation of this has been borrowed from Musl Libc
#ifndef incl_HPHP_UTIL_PORTABILITY_FNMATCH_H_
#define incl_HPHP_UTIL_PORTABILITY_FNMATCH_H_

#define	FNM_NOMATCH 1

#define	FNM_NOESCAPE 0x01
#define	FNM_PATHNAME 0x02
#define	FNM_PERIOD 0x04
#define	FNM_LEADING_DIR 0x08
#define	FNM_CASEFOLD 0x10
#define FNM_PREFIX_DIRS 0x20

extern "C" int fnmatch(const char *pattern, const char *string, int flags);

#endif
