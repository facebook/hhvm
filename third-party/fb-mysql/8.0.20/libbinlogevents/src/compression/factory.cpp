/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <compression/factory.h>
#include <compression/none.h>
#include <compression/zstd.h>
#include <my_byteorder.h>
#include <algorithm>

namespace binary_log {
namespace transaction {
namespace compression {

std::unique_ptr<Compressor> Factory::build_compressor(type t) {
  std::unique_ptr<Compressor> res{nullptr};
  switch (t) {
    case ZSTD:
      res = std::make_unique<Zstd_comp>();
      break;
    case NONE:
      res = std::make_unique<None_comp>();
      break;
    default:
      break;
  }
  return res;
}

std::unique_ptr<Decompressor> Factory::build_decompressor(type t) {
  std::unique_ptr<Decompressor> res{nullptr};
  switch (t) {
    case ZSTD:
      res = std::make_unique<Zstd_dec>();
      break;
    case NONE:
      res = std::make_unique<None_dec>();
      break;
    default:
      break;
  }
  return res;
}

}  // namespace compression
}  // namespace transaction
}  // namespace binary_log
