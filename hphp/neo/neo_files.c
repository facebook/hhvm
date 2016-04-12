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

#include "cs_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <io.h>
#include <windows.h>
#include <shellapi.h>
#include <strsafe.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

#include "neo_misc.h"
#include "neo_err.h"
#include "neo_files.h"

NEOERR *ne_load_file_len (const char *path, char **str, int *out_len)
{
  struct stat s;
  int fd;
  int len;
  int bytes_read;

  *str = NULL;
  if (out_len) *out_len = 0;

  if (stat(path, &s) == -1)
  {
    if (errno == ENOENT)
      return nerr_raise (NERR_NOT_FOUND, "File %s not found", path);
    return nerr_raise_errno (NERR_SYSTEM, "Unable to stat file %s", path);
  }

  if (s.st_size >= INT_MAX)
    return nerr_raise (NERR_ASSERT, "File %s too large (%ld >= INT_MAX)",
                       path, s.st_size);

  if (s.st_size < 0)
    return nerr_raise (NERR_ASSERT, "File %s size error? (%ld < 0)", path,
                       s.st_size);

  fd = open (path, O_RDONLY);
  if (fd == -1)
  {
    return nerr_raise_errno (NERR_SYSTEM, "Unable to open file %s", path);
  }
  len = s.st_size;
  *str = (char *) malloc (len + 1);

  if (*str == NULL)
  {
    close(fd);
    return nerr_raise (NERR_NOMEM,
	"Unable to allocate memory (%d) to load file %s", len + 1, path);
  }
  if ((bytes_read = read (fd, *str, len)) == -1)
  {
    close(fd);
    free(*str);
    return nerr_raise_errno (NERR_SYSTEM, "Unable to read file %s", path);
  }

  (*str)[bytes_read] = '\0';
  close(fd);
  if (out_len) *out_len = bytes_read;

  return STATUS_OK;
}

NEOERR *ne_load_file (const char *path, char **str) {
  return ne_load_file_len (path, str, NULL);
}
