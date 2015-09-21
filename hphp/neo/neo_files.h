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

#ifndef incl_HPHP_NEO_FILES_H_
#define incl_HPHP_NEO_FILES_H_ 1

__BEGIN_DECLS

#include <stdarg.h>
#include <sys/types.h>

NEOERR *ne_load_file (const char *path, char **str);
NEOERR *ne_load_file_len (const char *path, char **str, int *len);

__END_DECLS

#endif /* incl_HPHP_NEO_FILES_H_ */
