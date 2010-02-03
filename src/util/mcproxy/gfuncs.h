#ifndef __GFUNCS_H__
#define __GFUNCS_H__ 1

#include "include.h"

/*
 * Guarded memory allocation functions.
 */
extern void *  gmalloc(int size);
extern void *  gmalloc_data(int size);
extern void *  grealloc(void *orig, int size);
extern char *  gstrdup(const char *str);
extern char *  gstrdup_const(const char *str);
extern void *  gcalloc(int size);
extern void *  gcalloc_data(int size);
extern void *  gmemdup_const(const char *data, unsigned int length);
extern void  gfree(void *ptr);
extern void  ginit(void);

/*
 * Guarded string functions. These are safe to pass NULL pointers to.
 */
extern int  gstrlen(const char *str);
extern int  gatoi(const char *str);

/* gstrncpy always null-terminates */
extern void  gstrncpy(char *dest, const char *src, int maxlen);

/* convenience method to check for a string suffix */
extern int  endswith(const char *str, const char *suffix);

#endif /* __GFUNCS_H__ */
