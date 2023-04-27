/*****************************************************************************

Copyright (c) 2016, 2017, Oracle and/or its affiliates. All Rights Reserved.

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

#ifndef _buf0buf_h_
#define _buf0buf_h_

#include "fil0fil.h"
#include "fil0types.h"
#include "lot0types.h"

inline void buf_ptr_get_fsp_addr(const void *ptr, space_id_t *space,
                                 fil_addr_t *addr) {
  const page_t *page = (const page_t *)ut_align_down(ptr, UNIV_PAGE_SIZE);

  *space = mach_read_from_4(page + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);
  addr->page = mach_read_from_4(page + FIL_PAGE_OFFSET);
  addr->boffset = ut_align_offset(ptr, UNIV_PAGE_SIZE);
}

#endif  // _buf0buf_h_
