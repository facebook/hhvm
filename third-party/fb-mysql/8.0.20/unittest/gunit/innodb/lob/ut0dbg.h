/*****************************************************************************

Copyright (c) 2016, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/
#ifndef _ut0dbg_h_
#define _ut0dbg_h_

#include "lot0types.h"

void ut_dbg_assertion_failed(const char *expr, const char *file, ulint line);

#define ut_a(EXPR)                                               \
  do {                                                           \
    if (!(ulint)(EXPR)) {                                        \
      ut_dbg_assertion_failed(#EXPR, __FILE__, (ulint)__LINE__); \
    }                                                            \
  } while (0)

/** Abort execution. */
#define ut_error ut_dbg_assertion_failed(0, __FILE__, (ulint)__LINE__)

/** Debug assertion. Does nothing unless UNIV_DEBUG is defined. */
#define ut_ad(EXPR) ut_a(EXPR)
/** Debug statement. Does nothing unless UNIV_DEBUG is defined. */
#define ut_d(EXPR) EXPR

#endif  // _ut0dbg_h_
