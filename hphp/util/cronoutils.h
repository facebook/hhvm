/* ====================================================================
 * Copyright (c) 1995-1999 The Apache Group.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache"
 *    nor may "Apache" appear in their names without prior written
 *    permission of the Apache Group.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on public domain software written at the National Center for
 * Supercomputing Applications, University of Illinois, Urbana-Champaign.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 *
 */
/*
 * cronoutils -- utilities for the cronolog program
 *
 * Copyright (c) 1996-1999 by Ford & Mason Ltd
 *
 * This software was submitted by Ford & Mason Ltd to the Apache
 * Software Foundation in December 1999.  Future revisions and
 * derivatives of this source code must acknowledge Ford & Mason Ltd
 * as the original contributor of this module.  All other licensing
 * and usage conditions are those of the Apache Software Foundation.
 *
 * Originally written by Andrew Ford <A.Ford@ford-mason.co.uk>
 *
 * For platforms that don't declare getopt() in header files the symbol
 * NEED_GETOPT_DEFS can be defined and declarations are provided here.
 */

#ifndef incl_HPHP_CRONOUTILS_H_
#define incl_HPHP_CRONOUTILS_H_

/* Header files */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#include <direct.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef _WIN32
#define mode_t int

#define open  _open
#define close _close
#define read  _read
#define write _write
#define mkdir _mkdir
#endif

#if !HAVE_LOCALIME_R
struct tm *localtime_r(const time_t *, struct tm *) throw ();
#endif

/* Some operating systems don't declare getopt() */

#ifdef NEED_GETOPT_DEFS
int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;
#endif


/* If log files are not rotated then this is when the first file
 * should be closed. */

#define FAR_DISTANT_FUTURE  LONG_MAX


/* How often the log is rotated */

typedef enum
{
  PER_SECOND,
  PER_MINUTE,
  HOURLY,
  DAILY,
  WEEKLY,
  MONTHLY,
  YEARLY,
  ONCE_ONLY,
  UNKNOWN,
  INVALID_PERIOD
}
PERIODICITY;


/* Function prototypes */

void    create_subdirs(char *);
void    create_link(const char *, const char *, mode_t, const char *);
PERIODICITY  determine_periodicity(char *);
PERIODICITY   parse_timespec(char *optarg, int *p_period_multiple);
time_t    start_of_next_period(time_t, PERIODICITY, int);
time_t    start_of_this_period(time_t, PERIODICITY, int);
void    print_debug_msg(const char *msg, ...);
time_t    parse_time(char *time_str, int);
char     *timestamp(time_t thetime);


/* Global variables */

extern FILE  *debug_file;
extern const char  *periods[];
extern int  period_seconds[];


/* Usage message and CRONO_DEBUG macro.
 */

#ifdef CRONO_DEBUG
#undef CRONO_DEBUG
#endif

#define CRONO_DEBUG(msg_n_args)  \
do { if (debug_file) print_debug_msg  msg_n_args; } while (0)

#endif
