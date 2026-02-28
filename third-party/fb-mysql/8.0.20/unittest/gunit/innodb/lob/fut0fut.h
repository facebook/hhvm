/*****************************************************************************

Copyright (c) 2016, 2019, Oracle and/or its affiliates. All Rights Reserved.

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
#ifndef _fut0fut_h_
#define _fut0fut_h_

#include "fil0fil.h"
#include "lot0buf.h"

inline byte *fut_get_ptr(fil_addr_t addr, buf_block_t **ptr_block = nullptr) {
  buf_block_t *block;
  byte *ptr;

  ut_ad(addr.boffset < UNIV_PAGE_SIZE);

  block = buf_page_get(addr.page);
  ptr = buf_block_get_frame(block) + addr.boffset;

  if (ptr_block != nullptr) {
    *ptr_block = block;
  }

  return (ptr);
}

#endif  // _fut0fut_h_
