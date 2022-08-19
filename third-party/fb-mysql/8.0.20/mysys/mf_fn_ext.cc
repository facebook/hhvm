/* Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/mf_fn_ext.cc
*/

#include <string.h>

#include "m_string.h"
#include "my_dbug.h"
#include "my_io.h"
#if defined(FN_DEVCHAR) || defined(_WIN32)
#include "my_sys.h"
#include "mysys/mysys_priv.h"
#endif

/*
  Return a pointer to the extension of the filename.

  SYNOPSIS
    fn_ext()
    name		Name of file

  DESCRIPTION
    The extension is defined as everything after the last extension character
    (normally '.') after the directory name.

  RETURN VALUES
    Pointer to to the extension character. If there isn't any extension,
    points at the end ASCII(0) of the filename.
*/

const char *fn_ext(const char *name) {
#if defined(FN_DEVCHAR) || defined(_WIN32)
  char buff[FN_REFLEN];
  size_t res_length;
  const char *gpos = name + dirname_part(buff, name, &res_length);
#else
  const char *gpos = strrchr(name, FN_LIBCHAR);
  if (gpos == nullptr) gpos = name;
#endif
  const char *pos = strrchr(gpos, FN_EXTCHAR);
  return pos ? pos : strend(gpos);
}

char *fn_ext(char *name) {
  return const_cast<char *>(fn_ext(static_cast<const char *>(name)));
}
