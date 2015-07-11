#ifndef incl_HPHP_NEO_BOOL_H_
#define incl_HPHP_NEO_BOOL_H_ 1

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef _MSC_VER
typedef int BOOL;
#else
typedef char BOOL;
#endif

#endif /* incl_HPHP_NEO_BOOL_H_ */
