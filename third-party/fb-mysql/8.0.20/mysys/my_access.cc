/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_access.cc
*/

#include <errno.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"  // IWYU pragma: keep

#ifdef _WIN32

#include "my_io.h"
#include "my_thread_local.h"

/*
  Check a file or path for accessability.

  SYNOPSIS
    file_access()
    path 	Path to file
    amode	Access method

  DESCRIPTION
    This function wraps the normal access method because the access
    available in MSVCRT> +reports that filenames such as LPT1 and
    COM1 are valid (they are but should not be so for us).

  RETURN VALUES
  0    ok
  -1   error  (We use -1 as my_access is mapped to access on other platforms)
*/

int my_access(const char *path, int amode) {
  WIN32_FILE_ATTRIBUTE_DATA fileinfo;
  BOOL result;

  result = GetFileAttributesEx(path, GetFileExInfoStandard, &fileinfo);
  if (!result ||
      (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY) && (amode & W_OK)) {
    errno = EACCES;
    set_my_errno(EACCES);
    return -1;
  }
  return 0;
}

#endif /* _WIN32 */

/*
  List of file names that causes problem on windows

  NOTE that one can also not have file names of type CON.TXT

  NOTE: it is important to keep "CLOCK$" on the first place,
  we skip it in check_if_legal_tablename.
*/
static const char *reserved_names[] = {
    "CLOCK$", "CON",  "PRN",  "AUX",  "NUL",  "COM1", "COM2", "COM3",
    "COM4",   "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2",
    "LPT3",   "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9", NullS};

/*
  Looks up a null-terminated string in a list,
  case insensitively.

  SYNOPSIS
    str_list_find()
    list        list of items
    str         item to find

  RETURN
    0  ok
    1  reserved file name
*/
static int str_list_find(const char **list, const char *str) {
  const char **name;
  for (name = list; *name; name++) {
    if (!my_strcasecmp(&my_charset_latin1, *name, str)) return 1;
  }
  return 0;
}

/*
  A map for faster reserved_names lookup,
  helps to avoid loops in many cases.
  1 - can be the first letter
  2 - can be the second letter
  4 - can be the third letter
*/
static char reserved_map[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*  !"#$%&'()*+,-./ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0123456789:;<=>? */
    0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 5, 2, /* @ABCDEFGHIJKLMNO */
    3, 0, 2, 0, 4, 2, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, /* PQRSTUVWXYZ[\]^_ */
    0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 5, 2, /* bcdefghijklmno */
    3, 0, 2, 0, 4, 2, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, /* pqrstuvwxyz{|}~. */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ................ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /* ................ */
};

/*
  Check if a table name may cause problems

  SYNOPSIS
    check_if_legal_tablename
    name 	Table name (without any extensions)

  DESCRIPTION
    We don't check 'CLOCK$' because dollar sign is encoded as @0024,
    making table file name 'CLOCK@0024', which is safe.
    This is why we start lookup from the second element
    (i.e. &reserver_name[1])

  RETURN
    0  ok
    1  reserved file name
*/

int check_if_legal_tablename(const char *name) {
  DBUG_TRACE;
  return name[0] != 0 && name[1] != 0 && (reserved_map[(uchar)name[0]] & 1) &&
         (reserved_map[(uchar)name[1]] & 2) &&
         (reserved_map[(uchar)name[2]] & 4) &&
         str_list_find(&reserved_names[1], name);
}

#ifdef _WIN32
/**
  Checks if the drive letter supplied is valid or not. Valid drive
  letters are A to Z, both lower case and upper case.

  @param drive_letter : The drive letter to validate.

  @return true if the drive exists, false otherwise.
*/
static bool does_drive_exists(char drive_letter) {
  DWORD drive_mask = GetLogicalDrives();
  drive_letter = toupper(drive_letter);

  return (drive_letter >= 'A' && drive_letter <= 'Z') &&
         (drive_mask & (0x1 << (drive_letter - 'A')));
}

/**
  Verifies if the file name supplied is allowed or not. On Windows
  file names with a colon (:) are not allowed because such file names
  store data in Alternate Data Streams which can be used to hide
  the data.

  @param name contains the file name with or without path
  @param length contains the length of file name
  @param allow_current_dir true if paths like C:foobar are allowed,
                           false otherwise

  @return true if the file name is allowed, false otherwise.
*/
bool is_filename_allowed(const char *name MY_ATTRIBUTE((unused)),
                         size_t length MY_ATTRIBUTE((unused)),
                         bool allow_current_dir MY_ATTRIBUTE((unused))) {
  /*
    For Windows, check if the file name contains : character.
    Start from end of path and search if the file name contains :
  */
  const char *ch = NULL;
  for (ch = name + length - 1; ch >= name; --ch) {
    if (FN_LIBCHAR == *ch || '/' == *ch)
      break;
    else if (':' == *ch) {
      /*
        File names like C:foobar.txt are allowed since the syntax means
        file foobar.txt in current directory of C drive. However file
        names likes CC:foobar are not allowed since this syntax means ADS
        foobar in file CC.
      */
      return (allow_current_dir && (ch - name == 1) &&
              does_drive_exists(*name));
    }
  }
  return true;
} /* is_filename_allowed */
#endif /* _WIN32 */

#if defined(_WIN32)

/*
  Check if a path will access a reserverd file name that may cause problems

  SYNOPSIS
    check_if_legal_filename
    path 	Path to file

  RETURN
    0  ok
    1  reserved file name
*/

#define MAX_RESERVED_NAME_LENGTH 6

int check_if_legal_filename(const char *path) {
  const char *end;
  const char **reserved_name;
  DBUG_TRACE;

  if (!is_filename_allowed(path, strlen(path), true)) return 1;

  path += dirname_length(path); /* To start of filename */
  if (!(end = strchr(path, FN_EXTCHAR))) end = strend(path);
  if (path == end || (uint)(end - path) > MAX_RESERVED_NAME_LENGTH)
    return 0; /* Simplify inner loop */

  for (reserved_name = reserved_names; *reserved_name; reserved_name++) {
    const char *reserved = *reserved_name; /* never empty */
    const char *name = path;

    do {
      if (*reserved != my_toupper(&my_charset_latin1, *name)) break;
      if (++name == end && !reserved[1]) return 1; /* Found wrong path */
    } while (*++reserved);
  }
  return 0;
}

#endif /* defined(_WIN32) */
