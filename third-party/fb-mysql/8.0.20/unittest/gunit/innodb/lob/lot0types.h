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
#ifndef _lot0types_h_
#define _lot0types_h_

#include <stdint.h>
#include <cassert>
#include <iostream>
#include <limits>

using ulint = unsigned long int;
using lobid_t = unsigned long int;
using byte = unsigned char;
using frag_id_t = ulint;
typedef uint32_t space_id_t;
typedef uint64_t ib_uint64_t;
typedef ib_uint64_t ib_id_t;
using page_no_t = uint32_t;
typedef byte page_t;
typedef ib_id_t trx_id_t;

const ulint FRAG_ID_NULL = std::numeric_limits<uint16_t>::max();
const ulint KB1 = 1024;
const ulint KB2 = 2 * 1024;
const ulint KB5 = 5 * 1024;
const ulint KB16 = 16 * 1024;
const ulint KB64 = 64 * 1024;
const ulint KB128 = 128 * 1024;
const ulint KB300 = 300 * 1024;
const ulint KB500 = 500 * 1024;
const ulint MB16 = 16 * 1024 * 1024;
const ulint MB10 = 10 * 1024 * 1024;
const ulint MB1 = 1024 * 1024;
const ulint MB2 = 2 * MB1;
const ulint MB3 = 3 * MB1;
const ulint MB4 = 2 * MB2;
const ulint MB5 = 5 * 1024 * 1024;
const ulint MB7 = 7 * 1024 * 1024;
const ulint MB100 = 100 * 1024 * 1024;
const ulint GB1 = 1024 * 1024 * 1024 * 1UL;
const uint64_t GB4 = 1024 * 1024 * 1024 * 4UL;
const ulint FIL_PAGE_DATA = 38U;
const unsigned int UNIV_PAGE_SIZE = 16384; /* 16KB */
constexpr page_no_t FIL_NULL = std::numeric_limits<page_no_t>::max();

/** page offset inside space */
#define FIL_PAGE_OFFSET 4

/** The next page information stored in 4 bytes. */
#define FIL_PAGE_NEXT 12
#define FIL_PAGE_TYPE 24

/** start of the data on the page */
#define FIL_PAGE_DATA 38U

/** The bitmask of 32-bit unsigned integer */
#define ULINT32_MASK 0xFFFFFFFF

#define FIL_ADDR_SIZE 6 /* address size is 6 bytes */

/* The physical size of a list base node in bytes */
#define FLST_BASE_NODE_SIZE (4 + 2 * FIL_ADDR_SIZE)

/* The physical size of a list node in bytes */
#define FLST_NODE_SIZE (2 * FIL_ADDR_SIZE)

#endif  // _lot0types_h_
