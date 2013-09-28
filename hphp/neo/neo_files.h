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
#include "hphp/neo/ulist.h"



typedef int (* MATCH_FUNC)(void *rock, const char *filename);

NEOERR *ne_mkdirs (const char *path, mode_t mode);
NEOERR *ne_load_file (const char *path, char **str);
NEOERR *ne_load_file_len (const char *path, char **str, int *len);
NEOERR *ne_save_file (const char *path, char *str);
NEOERR *ne_remove_dir (const char *path);
NEOERR *ne_listdir(const char *path, ULIST **files);
NEOERR *ne_listdir_match(const char *path, ULIST **files, const char *match);
NEOERR *ne_listdir_fmatch(const char *path, ULIST **files, MATCH_FUNC fmatch,
                          void *rock);

__END_DECLS

#endif /* incl_HPHP_NEO_FILES_H_ */
