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

#include <codecs/binary.h>
#include <codecs/factory.h>

namespace binary_log {
namespace codecs {

std::unique_ptr<Codec> Factory::build_codec(Log_event_type t) {
  switch (t) {
    case TRANSACTION_PAYLOAD_EVENT:
      return std::make_unique<
          binary_log::codecs::binary::Transaction_payload>();
    default:              /* purecov: inspected */
      BAPI_ASSERT(false); /* purecov: inspected */
  }

  return nullptr;
}

}  // namespace codecs
}  // namespace binary_log
