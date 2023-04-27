/*
 * Portions Copyright (c) 1987, 1993, 1994
 * The Regents of the University of California.  All rights reserved.
 *
 * Portions Copyright (c) 2003-2010, PostgreSQL Global Development Group
 */

#pragma once

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

extern int opterr;
extern int optind;
extern int optopt;
extern char* optarg;

struct option {
  const char* name;
  int has_arg;
  int* flag;
  int val;
};

#define no_argument 0
#define required_argument 1

extern int getopt_long(
    int argc,
    char* const argv[],
    const char* optstring,
    const struct option* longopts,
    int* longindex);

int getopt(int nargc, char* const nargv[], const char* ostr);

#ifdef __cplusplus
}
#endif

#endif
