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

#include "codecs/binary.h"
#include <codecs/base.h>
#include <string>
#include "my_byteorder.h"
#include "mysql_com.h"  // net_store_length, net_length_size

namespace binary_log {
namespace codecs {
namespace binary {

/**
  This is the decoder member function for the TLV encoded set of
  values for the Transaction payload event. Note that the log event
  must have the appropriate type code: TRANSACTION_PAYLOAD_EVENT to
  be compatible with this specific decoder. If it is not, an error
  shall be returned

  @param from the buffer to decode from.
  @param size the buffer size.
  @param to The log event to decode to.

  @return a pair containing the number of bytes decoded and whether
          there was an error or not. False if no error, true
          otherwise.
 */
std::pair<std::size_t, bool> Transaction_payload::decode(
    const unsigned char *from, size_t size, Binary_log_event &to) const {
  bool result =
      (to.header()->type_code != binary_log::TRANSACTION_PAYLOAD_EVENT);
  Event_reader reader(reinterpret_cast<const char *>(from), size);
  BAPI_ASSERT(!result);
  if (!result) {
    Transaction_payload_event &ev = down_cast<Transaction_payload_event &>(to);

    while (reader.available_to_read()) {
      uint64_t type{0};
      uint64_t length{0};
      uint64_t value{0};

      /* read the type of the field. */
      if (reader.can_read(UINT_64T_MIN_SIZE)) {
        value = reader.net_field_length_ll();
        if ((result = reader.get_error())) goto end;
        type = value;
      }

      // we have reached the end of the header
      if (type == Transaction_payload_event::OTW_PAYLOAD_HEADER_END_MARK) break;

      /* read the size of the field. */
      if (reader.can_read(UINT_64T_MIN_SIZE)) {
        value = reader.net_field_length_ll();
        if ((result = reader.get_error())) goto end;
        length = value;
      }

      switch (type) {
        case Transaction_payload_event::OTW_PAYLOAD_SIZE_FIELD:
          /* fetch the payload size */
          if (reader.can_read(UINT_64T_MIN_SIZE)) {
            value = reader.net_field_length_ll();
            if ((result = reader.get_error())) goto end;
            ev.set_payload_size(value);
          }
          break;

        case Transaction_payload_event::OTW_PAYLOAD_COMPRESSION_TYPE_FIELD:
          /* fetch the compression type */
          if (reader.can_read(UINT_64T_MIN_SIZE)) {
            value = reader.net_field_length_ll();
            if ((result = reader.get_error())) goto end;
            ev.set_compression_type((transaction::compression::type)value);
          }
          break;

        case Transaction_payload_event::OTW_PAYLOAD_UNCOMPRESSED_SIZE_FIELD:
          /* fetch the uncompressed size */
          if (reader.can_read(UINT_64T_MIN_SIZE)) {
            value = reader.net_field_length_ll();
            if ((result = reader.get_error())) goto end;
            ev.set_uncompressed_size(value);
          }
          break;

        /* purecov: begin inspected */
        default:
          /* ignore unrecognized field. */
          reader.forward(length);
          break;
          /* purecov: end */
      }
    }

    // ----------------------------------------------------
    // Handle defaults here.
    // ----------------------------------------------------

    if (ev.get_uncompressed_size() == 0)
      // default the uncompressed to the payload size
      ev.set_uncompressed_size(ev.get_payload_size()); /* purecov: inspected */
  }

end:
  return std::make_pair<std::size_t, bool &>(reader.position(), result);
}

/**
  This is the encoder member function for the TLV encoded set of
  values for the Transaction payload event. Note that the log event
  must have the appropriate type code: TRANSACTION_PAYLOAD_EVENT to
  be compatible with this specific enccoder. If it is not, an error
  shall be returned

  @param from The log event to decode to.
  @param to the buffer to decode from.
  @param size the buffer size.

  @return a pair containing the number of bytes decoded and whether
          there was an error or not. False if no error, true
          otherwise.
 */
std::pair<std::size_t, bool> Transaction_payload::encode(
    const Binary_log_event &from, unsigned char *to,
    size_t size MY_ATTRIBUTE((unused))) const {
  bool result =
      from.header()->type_code != binary_log::TRANSACTION_PAYLOAD_EVENT;
  uchar *ptr = to;
  BAPI_ASSERT(!result);
  if (result == false) {
    uint64_t type{0};
    uint64_t length{0};
    uint64_t value{0};
    const Transaction_payload_event &ev =
        down_cast<const Transaction_payload_event &>(from);

    // encode the compression type
    type = static_cast<uint64_t>(
        Transaction_payload_event::OTW_PAYLOAD_COMPRESSION_TYPE_FIELD);
    value = static_cast<uint64_t>(ev.get_compression_type());
    length = net_length_size(value);
    ptr = net_store_length(ptr, type);
    ptr = net_store_length(ptr, length);
    ptr = net_store_length(ptr, value);

    // encode uncompressed size
    if (ev.get_compression_type() != transaction::compression::type::NONE) {
      type = static_cast<uint64_t>(
          Transaction_payload_event::OTW_PAYLOAD_UNCOMPRESSED_SIZE_FIELD);
      value = ev.get_uncompressed_size();
      length = net_length_size(value);
      ptr = net_store_length(ptr, type);
      ptr = net_store_length(ptr, length);
      ptr = net_store_length(ptr, value);
    }

    // encode payload size
    type = static_cast<uint64_t>(
        Transaction_payload_event::OTW_PAYLOAD_SIZE_FIELD);
    value = ev.get_payload_size();
    length = net_length_size(value);
    ptr = net_store_length(ptr, type);
    ptr = net_store_length(ptr, length);
    ptr = net_store_length(ptr, value);

    // write the end of event header marker
    type = static_cast<uint64_t>(
        Transaction_payload_event::OTW_PAYLOAD_HEADER_END_MARK);
    ptr = net_store_length(ptr, type);
  }

  return std::make_pair<std::size_t, bool &>(ptr - to, result);
}

}  // namespace binary
}  // namespace codecs
}  // namespace binary_log