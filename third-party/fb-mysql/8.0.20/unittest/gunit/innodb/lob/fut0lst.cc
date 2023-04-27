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

#include "fut0lst.h"
#include "buf0buf.h"
#include "fil0fil.h"
#include "fut0fut.h"
#include "mtr0log.h"
#include "page0page.h"

void flst_write_addr(fil_faddr_t *faddr, fil_addr_t addr) {
  mlog_write_ulint(faddr + FIL_ADDR_PAGE, addr.page, MLOG_4BYTES);
  mlog_write_ulint(faddr + FIL_ADDR_BYTE, addr.boffset, MLOG_2BYTES);
}

void flst_init(flst_base_node_t *base) {
  mlog_write_ulint(base + FLST_LEN, 0, MLOG_4BYTES);
  flst_write_addr(base + FLST_FIRST, fil_addr_null);
  flst_write_addr(base + FLST_LAST, fil_addr_null);
}

void flst_insert_after(flst_base_node_t *base, flst_node_t *node1,
                       flst_node_t *node2) {
  space_id_t space;
  fil_addr_t node1_addr;
  fil_addr_t node2_addr;
  flst_node_t *node3;
  fil_addr_t node3_addr;
  ulint len;

  ut_ad(base != node1);
  ut_ad(base != node2);
  ut_ad(node2 != node1);

  buf_ptr_get_fsp_addr(node1, &space, &node1_addr);
  buf_ptr_get_fsp_addr(node2, &space, &node2_addr);

  node3_addr = flst_get_next_addr(node1);

  /* Set prev and next fields of node2 */
  flst_write_addr(node2 + FLST_PREV, node1_addr);
  flst_write_addr(node2 + FLST_NEXT, node3_addr);

  if (!fil_addr_is_null(node3_addr)) {
    /* Update prev field of node3 */
    node3 = fut_get_ptr(node3_addr);
    flst_write_addr(node3 + FLST_PREV, node2_addr);
  } else {
    /* node1 was last in list: update last field in base */
    flst_write_addr(base + FLST_LAST, node2_addr);
  }

  /* Set next field of node1 */
  flst_write_addr(node1 + FLST_NEXT, node2_addr);

  /* Update len of base node */
  len = flst_get_len(base);
  mlog_write_ulint(base + FLST_LEN, len + 1, MLOG_4BYTES);
}

static void flst_add_to_empty(flst_base_node_t *base, flst_node_t *node) {
  space_id_t space;
  fil_addr_t node_addr;
  ulint len;

  ut_ad(base != node);

  len = flst_get_len(base);
  ut_a(len == 0);

  buf_ptr_get_fsp_addr(node, &space, &node_addr);

  /* Update first and last fields of base node */
  flst_write_addr(base + FLST_FIRST, node_addr);
  flst_write_addr(base + FLST_LAST, node_addr);

  /* Set prev and next fields of node to add */
  flst_write_addr(node + FLST_PREV, fil_addr_null);
  flst_write_addr(node + FLST_NEXT, fil_addr_null);

  /* Update len of base node */
  mlog_write_ulint(base + FLST_LEN, len + 1, MLOG_4BYTES);
}

void flst_add_last(flst_base_node_t *base, flst_node_t *node) {
  space_id_t space;
  fil_addr_t node_addr;
  ulint len;
  fil_addr_t last_addr;

  ut_ad(base != node);
  len = flst_get_len(base);
  last_addr = flst_get_last(base);

  buf_ptr_get_fsp_addr(node, &space, &node_addr);

  /* If the list is not empty, call flst_insert_after */
  if (len != 0) {
    flst_node_t *last_node;

    if (last_addr.page == node_addr.page) {
      last_node = page_align(node) + last_addr.boffset;
    } else {
      last_node = fut_get_ptr(last_addr);
    }

    flst_insert_after(base, last_node, node);
  } else {
    /* else call flst_add_to_empty */
    flst_add_to_empty(base, node);
  }
}

bool flst_validate(const flst_base_node_t *base) {
  space_id_t space;
  const flst_node_t *node;
  fil_addr_t node_addr;
  fil_addr_t base_addr;
  ulint len;
  ulint i;

  ut_ad(base);

  buf_ptr_get_fsp_addr(base, &space, &base_addr);

  len = flst_get_len(base);
  node_addr = flst_get_first(base);

  for (i = 0; i < len; i++) {
    node = fut_get_ptr(node_addr);
    node_addr = flst_get_next_addr(node);
  }

  ut_a(fil_addr_is_null(node_addr));

  node_addr = flst_get_last(base);

  for (i = 0; i < len; i++) {
    node = fut_get_ptr(node_addr);
    node_addr = flst_get_prev_addr(node);
  }

  ut_a(fil_addr_is_null(node_addr));

  return (true);
}

void flst_remove(flst_base_node_t *base, flst_node_t *node2) {
  space_id_t space;
  flst_node_t *node1;
  fil_addr_t node1_addr;
  fil_addr_t node2_addr;
  flst_node_t *node3;
  fil_addr_t node3_addr;
  ulint len;

  buf_ptr_get_fsp_addr(node2, &space, &node2_addr);

  node1_addr = flst_get_prev_addr(node2);
  node3_addr = flst_get_next_addr(node2);

  if (!fil_addr_is_null(node1_addr)) {
    /* Update next field of node1 */

    if (node1_addr.page == node2_addr.page) {
      node1 = page_align(node2) + node1_addr.boffset;
    } else {
      node1 = fut_get_ptr(node1_addr);
    }

    ut_ad(node1 != node2);

    flst_write_addr(node1 + FLST_NEXT, node3_addr);
  } else {
    /* node2 was first in list: update first field in base */
    flst_write_addr(base + FLST_FIRST, node3_addr);
  }

  if (!fil_addr_is_null(node3_addr)) {
    /* Update prev field of node3 */

    if (node3_addr.page == node2_addr.page) {
      node3 = page_align(node2) + node3_addr.boffset;
    } else {
      node3 = fut_get_ptr(node3_addr);
    }

    ut_ad(node2 != node3);

    flst_write_addr(node3 + FLST_PREV, node1_addr);
  } else {
    /* node2 was last in list: update last field in base */
    flst_write_addr(base + FLST_LAST, node1_addr);
  }

  /* Update len of base node */
  len = flst_get_len(base);
  ut_ad(len > 0);

  mlog_write_ulint(base + FLST_LEN, len - 1, MLOG_4BYTES);
}

void flst_insert_before(flst_base_node_t *base, flst_node_t *node2,
                        flst_node_t *node3) {
  space_id_t space;
  flst_node_t *node1;
  fil_addr_t node1_addr;
  fil_addr_t node2_addr;
  fil_addr_t node3_addr;
  ulint len;

  ut_ad(base != node2);
  ut_ad(base != node3);
  ut_ad(node2 != node3);

  buf_ptr_get_fsp_addr(node2, &space, &node2_addr);
  buf_ptr_get_fsp_addr(node3, &space, &node3_addr);

  node1_addr = flst_get_prev_addr(node3);

  /* Set prev and next fields of node2 */
  flst_write_addr(node2 + FLST_PREV, node1_addr);
  flst_write_addr(node2 + FLST_NEXT, node3_addr);

  if (!fil_addr_is_null(node1_addr)) {
    /* Update next field of node1 */
    node1 = fut_get_ptr(node1_addr);
    flst_write_addr(node1 + FLST_NEXT, node2_addr);
  } else {
    /* node3 was first in list: update first field in base */
    flst_write_addr(base + FLST_FIRST, node2_addr);
  }

  /* Set prev field of node3 */
  flst_write_addr(node3 + FLST_PREV, node2_addr);

  /* Update len of base node */
  len = flst_get_len(base);
  mlog_write_ulint(base + FLST_LEN, len + 1, MLOG_4BYTES);
}

/** Adds a node as the first node in a list.
@param[in]	base	pointer to base node of list.
@param[in]	node	pointer to node being added. */
void flst_add_first(flst_base_node_t *base, flst_node_t *node) {
  space_id_t space;
  fil_addr_t node_addr;
  ulint len;
  fil_addr_t first_addr;
  flst_node_t *first_node;

  ut_ad(base != node);
  len = flst_get_len(base);
  first_addr = flst_get_first(base);

  buf_ptr_get_fsp_addr(node, &space, &node_addr);

  /* If the list is not empty, call flst_insert_before */
  if (len != 0) {
    if (first_addr.page == node_addr.page) {
      first_node = page_align(node) + first_addr.boffset;
    } else {
      first_node = fut_get_ptr(first_addr);
    }

    flst_insert_before(base, node, first_node);
  } else {
    /* else call flst_add_to_empty */
    flst_add_to_empty(base, node);
  }
}
