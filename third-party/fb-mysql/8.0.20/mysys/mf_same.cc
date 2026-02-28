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
  @file mysys/mf_same.cc
*/

#include <string.h>

#include "my_dbug.h"
#include "my_io.h"
#include "my_sys.h"

/*
  Copy directory and/or extension between filenames.
  (For the meaning of 'flag', check mf_format.c)
  'to' may be equal to 'name'.
  Returns 'to'.
*/

char *fn_same(char *to, const char *name, int flag) {
  char dev[FN_REFLEN];
  const char *ext;
  size_t dev_length;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("to: %s  name: %s  flag: %d", to, name, flag));

  if ((ext = strrchr(name + dirname_part(dev, name, &dev_length),
                     FN_EXTCHAR)) == nullptr)
    ext = "";

  return fn_format(to, to, dev, ext, flag);
} /* fn_same */
