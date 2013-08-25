/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#ifndef incl_HPHP_NEO_ERR_H_
#define incl_HPHP_NEO_ERR_H_ 1

#include "hphp/neo/neo_misc.h"

/* For compilers (well, cpp actually) which don't define __PRETTY_FUNCTION__ */
#ifndef __GNUC__
#define __PRETTY_FUNCTION__ "unknown_function"
#endif

__BEGIN_DECLS

/* For 64 bit systems which don't like mixing ints and pointers, we have the
 * _INT version for doing that comparison */
#define STATUS_OK ((NEOERR *)0)
#define STATUS_OK_INT 0
#define INTERNAL_ERR ((NEOERR *)1)
#define INTERNAL_ERR_INT 1

/* NEOERR flags */
#define NE_IN_USE (1<<0)

typedef int NERR_TYPE;

/* Predefined Error Types - These are all registered in nerr_init */
extern NERR_TYPE NERR_PASS;
extern NERR_TYPE NERR_ASSERT;
extern NERR_TYPE NERR_NOT_FOUND;
extern NERR_TYPE NERR_DUPLICATE;
extern NERR_TYPE NERR_NOMEM;
extern NERR_TYPE NERR_PARSE;
extern NERR_TYPE NERR_OUTOFRANGE;
extern NERR_TYPE NERR_SYSTEM;
extern NERR_TYPE NERR_IO;
extern NERR_TYPE NERR_LOCK;
extern NERR_TYPE NERR_DB;
extern NERR_TYPE NERR_EXISTS;
extern NERR_TYPE NERR_MAX_RECURSION;

typedef struct _neo_err
{
  int error;
  int err_stack;
  int flags;
  char desc[256];
  const char *file;
  const char *func;
  int lineno;
  /* internal use only */
  struct _neo_err *next;
} NEOERR;

/* Technically, we could do this in configure and detect what their compiler
 * can handle, but for now... */
#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define USE_C99_VARARG_MACROS 1
#elif __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 4) || defined (S_SPLINT_S)
#define USE_GNUC_VARARG_MACROS 1
#else
#error The compiler is missing support for variable-argument macros.
#endif


/*
 * function: nerr_raise
 * description: Use this method to create an error "exception" for
 *              return up the call chain
 * arguments: using the macro, the function name, file, and lineno are
 *            automagically recorded for you.  You just provide the
 *            error (from those listed above) and the printf-style
 *            reason.  THIS IS A PRINTF STYLE FUNCTION, DO NOT PASS
 *            UNKNOWN STRING DATA AS THE FORMAT STRING.
 * returns: a pointer to a NEOERR, or INTERNAL_ERR if allocation of
 *          NEOERR fails
 */
#if defined(USE_C99_VARARG_MACROS)
#define nerr_raise(e,...) \
   nerr_raisef(__PRETTY_FUNCTION__,__FILE__,__LINE__,e,__VA_ARGS__)
#elif defined(USE_GNUC_VARARG_MACROS)
#define nerr_raise(e,f,a...) \
   nerr_raisef(__PRETTY_FUNCTION__,__FILE__,__LINE__,e,f,##a)
#endif

NEOERR *nerr_raisef (const char *func, const char *file, int lineno,
                     NERR_TYPE error, const char *fmt, ...)
                     ATTRIBUTE_PRINTF(5,6);



#if defined(USE_C99_VARARG_MACROS)
#define nerr_raise_errno(e,...) \
   nerr_raise_errnof(__PRETTY_FUNCTION__,__FILE__,__LINE__,e,__VA_ARGS__)
#elif defined(USE_GNUC_VARARG_MACROS)
#define nerr_raise_errno(e,a...) \
   nerr_raise_errnof(__PRETTY_FUNCTION__,__FILE__,__LINE__,e,##a)
#endif

NEOERR *nerr_raise_errnof (const char *func, const char *file, int lineno,
                           int error, const char *fmt, ...)
                           ATTRIBUTE_PRINTF(5,6);

/* function: nerr_pass - pass a clearsilver error up a level in the call chain
 * description: this function is used to pass an error up a level in the
 *              call chain (ie, if the error isn't handled at the
 *              current level).  This allows us to track the traceback
 *              of the error.
 * arguments: with the macro, the function name, file and lineno are
 *            automagically recorded.  Just pass the error.
 * returns: a pointer to an error
 */
#define nerr_pass(e) \
   nerr_passf(__PRETTY_FUNCTION__,__FILE__,__LINE__,e)

NEOERR *nerr_passf (const char *func, const char *file, int lineno,
                    NEOERR *nerr);

/* function: nerr_pass_ctx - pass a clearsilver error up a level in the call
 *           chain with additional information
 * description: this function is used to pass an error up a level in the
 *              call chain (ie, if the error isn't handled at the
 *              current level).  This allows us to track the traceback
 *              of the error.
 *              This version includes context information about lower
 *              errors
 * arguments: with the macro, the function name, file and lineno are
 *            automagically recorded.  Just pass the error and
 *            a printf format string giving more information about where
 *            the error is occuring.
 * returns: a pointer to an error
 */
#if defined(USE_C99_VARARG_MACROS)
#define nerr_pass_ctx(e,...) \
   nerr_pass_ctxf(__PRETTY_FUNCTION__,__FILE__,__LINE__,e,__VA_ARGS__)
#elif defined(USE_GNUC_VARARG_MACROS)
#define nerr_pass_ctx(e,a...) \
   nerr_pass_ctxf(__PRETTY_FUNCTION__,__FILE__,__LINE__,e,##a)
#endif

NEOERR *nerr_pass_ctxf (const char *func, const char *file, int lineno,
                        NEOERR *nerr, const char *fmt, ...)
                        ATTRIBUTE_PRINTF(5,6);

/* function: nerr_log_error - print the error chain to stderr
 * description: prints out the error traceback to stderr
 */
void nerr_log_error (NEOERR *nerr);

#include "hphp/neo/neo_str.h"
/* function: nerr_error_string - returns the string associated with an error
 * description: returns the string associated with an error (the bottom
 *              level of the error chain)
 * arguments: nerr - error
 *            str - string to which the data is appended
 * returns: None - errors appending to the string are ignored
 */
void nerr_error_string (NEOERR *nerr, NEOSTRING *str);

/* function: nerr_error_traceback - returns the full trackeback of the error
 *          chain
 * description: returns the full traceback of the error chain
 * arguments: nerr - error
 *            str - string to which the data is appended
 * returns: None - errors appending to the string are ignored
 */
void nerr_error_traceback (NEOERR *nerr, NEOSTRING *str);

/* function: nerr_ignore - free the error chain
 * description: you should only call this if you actually handle the
 *              error (should I rename it?).  Free's the error chain.
 */
void nerr_ignore (NEOERR **err);

/* function: nerr_register - register a NEOERR type
 * description: register an error type.  This will assign a numeric value
 *              to the type, and keep track of the "pretty name" for it.
 * arguments: err - pointer to a NERR_TYPE
 *            name - pretty name for the error type
 * returns: NERR_NOMEM on no memory
 */
NEOERR *nerr_register (NERR_TYPE *err, const char *name);

/* function: nerr_init - initialize the NEOERR error subsystem
 * description: initialize the NEOERR system.  Can be called more than once.
 *              This registers all of the built in error types as defined at
 *              the top of this file.  If you don't call this, all exceptions
 *              will be returned as UnknownError.
 * arguments: None
 * returns: possibly NERR_NOMEM, but somewhat unlikely.  Possibly an
 *          UnknownError if NERR_NOMEM hasn't been registered yet.
 */
NEOERR *nerr_init (void);

/* function: nerr_match - walk the NEOERR chain for a matching error ("catch")
 * description: nerr_match is used to walk the NEOERR chain and match
 *              the error against a specific error type.  In exception
 *              parlance, this would be the equivalent of "catch".
 *              Typically, you can just compare a NEOERR against STATUS_OK
 *              or just test for true if you are checking for any error.
 * arguments: nerr - the NEOERR that has an error.
 *            type - the NEOERR type, as registered with nerr_register
 * returns: true on match
 */
int nerr_match (NEOERR *nerr, NERR_TYPE type);

/* function: nerr_handle - walk the NEOERR chain for a matching error
 * description: nerr_handle is a convenience function.  It is the equivalent
 *              of nerr_match, but it will also deallocate the error chain
 *              on a match.
 * arguments: err - pointer to a pointer NEOERR
 *            type - the NEOERR type, as registered with nerr_register
 * returns: true on match
 */
int nerr_handle (NEOERR **err, NERR_TYPE type);

__END_DECLS

#endif /* incl_HPHP_NEO_ERR_H_ */
