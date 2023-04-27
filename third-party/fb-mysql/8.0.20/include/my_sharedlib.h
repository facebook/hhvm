/*
   Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MY_SHAREDLIB_INCLUDED
#define MY_SHAREDLIB_INCLUDED

/**
  @file include/my_sharedlib.h
  Functions related to handling of plugins and other dynamically loaded
  libraries.
*/

#if defined(_WIN32)
#define dlsym(lib, name) (void *)GetProcAddress((HMODULE)lib, name)
#define dlopen(libname, unused) LoadLibraryEx(libname, NULL, 0)
#define dlclose(lib) FreeLibrary((HMODULE)lib)
#define DLERROR_GENERATE(errmsg, error_number)                          \
  char win_errormsg[2048];                                              \
  if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error_number, 0,     \
                    win_errormsg, 2048, NULL)) {                        \
    char *ptr;                                                          \
    for (ptr = &win_errormsg[0] + strlen(win_errormsg) - 1;             \
         ptr >= &win_errormsg[0] && strchr("\r\n\t\0x20", *ptr); ptr--) \
      *ptr = 0;                                                         \
    errmsg = win_errormsg;                                              \
  } else                                                                \
    errmsg = ""
#define dlerror() ""
#define dlopen_errno GetLastError()

#else /* _WIN32 */

#ifndef MYSQL_ABI_CHECK
#include <dlfcn.h>
#include <errno.h>
#endif

#define DLERROR_GENERATE(errmsg, error_number) errmsg = dlerror()
#define dlopen_errno errno
#endif /* _WIN32 */

/*
  MYSQL_PLUGIN_IMPORT macro is used to export mysqld data
  (i.e variables) for usage in storage engine loadable plugins.
  Outside of Windows, it is dummy.
*/
#if (defined(_WIN32) && defined(MYSQL_DYNAMIC_PLUGIN))
#define MYSQL_PLUGIN_IMPORT __declspec(dllimport)
#else
#define MYSQL_PLUGIN_IMPORT
#endif

#endif  // MY_SHAREDLIB_INCLUDED
