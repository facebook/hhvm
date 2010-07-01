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
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "neo_misc.h"
#include "neo_err.h"
#include "neo_files.h"
#include "wildmat.h"

NEOERR *ne_mkdirs (const char *path, mode_t mode)
{
  char mypath[PATH_BUF_SIZE];
  int x;
  int r;

  strncpy (mypath, path, sizeof(mypath));
  x = strlen(mypath);
  if ((x < sizeof(mypath)) && (mypath[x-1] != '/'))
  {
    mypath[x] = '/';
    mypath[x+1] = '\0';
  }

  for (x = 1; mypath[x]; x++)
  {
    if (mypath[x] == '/')
    {
      mypath[x] = '\0';
#ifdef __MINGW32__
      /* Braindead MINGW32 doesn't just have a dummy argument for mode */
      r = mkdir (mypath);
#else
      r = mkdir (mypath, mode);
#endif

      if (r == -1 && errno != EEXIST)
      {
	return nerr_raise_errno(NERR_SYSTEM, "ne_mkdirs: mkdir(%s, %x) failed", mypath, mode);
      }
      mypath[x] = '/';
    }
  }
  return STATUS_OK;
}

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

NEOERR *ne_save_file (const char *path, char *str)
{
  NEOERR *err;
  int fd;
  int w, l;

  fd = open (path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
  if (fd == -1)
  {
    return nerr_raise_errno (NERR_IO, "Unable to create file %s", path);
  }
  l = strlen(str);
  w = write (fd, str, l);
  if (w != l)
  {
    err = nerr_raise_errno (NERR_IO, "Unable to write file %s", path);
    close (fd);
    return err;
  }
  close (fd);

  return STATUS_OK;
}

NEOERR *ne_remove_dir (const char *path)
{
  NEOERR *err;
  DIR *dp;
  struct stat s;
  struct dirent *de;
  char npath[PATH_BUF_SIZE];

  if (stat(path, &s) == -1)
  {
    if (errno == ENOENT) return STATUS_OK;
    return nerr_raise_errno (NERR_SYSTEM, "Unable to stat file %s", path);
  }
  if (!S_ISDIR(s.st_mode))
  {
    return nerr_raise (NERR_ASSERT, "Path %s is not a directory", path);
  }
  dp = opendir(path);
  if (dp == NULL)
    return nerr_raise_errno (NERR_IO, "Unable to open directory %s", path);
  while ((de = readdir (dp)) != NULL)
  {
    if (strcmp(de->d_name, ".") && strcmp(de->d_name, ".."))
    {
      snprintf (npath, sizeof(npath), "%s/%s", path, de->d_name);
      if (stat(npath, &s) == -1)
      {
	if (errno == ENOENT) continue;
	closedir(dp);
	return nerr_raise_errno (NERR_SYSTEM, "Unable to stat file %s", npath);
      }
      if (S_ISDIR(s.st_mode))
      {
	err = ne_remove_dir(npath);
	if (err) break;
      }
      else
      {
	if (unlink(npath) == -1)
	{
	  if (errno == ENOENT) continue;
	  closedir(dp);
	  return nerr_raise_errno (NERR_SYSTEM, "Unable to unlink file %s",
	      npath);
	}
      }
    }
  }
  closedir(dp);
  if (rmdir(path) == -1)
  {
    return nerr_raise_errno (NERR_SYSTEM, "Unable to rmdir %s", path);
  }
  return STATUS_OK;
}

NEOERR *ne_listdir(const char *path, ULIST **files)
{
  return nerr_pass(ne_listdir_fmatch(path, files, NULL, NULL));
}

static int _glob_match(void *rock, const char *filename)
{
  return wildmat(filename, rock);
}

NEOERR *ne_listdir_match(const char *path, ULIST **files, const char *match)
{
  return nerr_pass(ne_listdir_fmatch(path, files, _glob_match, (void *)match));
}

NEOERR *ne_listdir_fmatch(const char *path, ULIST **files, MATCH_FUNC fmatch,
                          void *rock)
{
  DIR *dp;
  struct dirent *de;
  ULIST *myfiles = NULL;
  NEOERR *err = STATUS_OK;

  if (files == NULL)
    return nerr_raise(NERR_ASSERT, "Invalid call to ne_listdir_fmatch");

  if (*files == NULL)
  {
    err = uListInit(&myfiles, 10, 0);
    if (err) return nerr_pass(err);
  }
  else
  {
    myfiles = *files;
  }

  if ((dp = opendir (path)) == NULL)
  {
    return nerr_raise_errno(NERR_IO, "Unable to opendir %s", path);
  }
  while ((de = readdir (dp)) != NULL)
  {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
      continue;

    if (fmatch != NULL && !fmatch(rock, de->d_name))
      continue;

    err = uListAppend(myfiles, strdup(de->d_name));
    if (err) break;
  }
  closedir(dp);
  if (err && *files == NULL)
  {
    uListDestroy(&myfiles, ULIST_FREE);
  }
  else if (*files == NULL)
  {
    *files = myfiles;
  }
  return nerr_pass(err);
}
