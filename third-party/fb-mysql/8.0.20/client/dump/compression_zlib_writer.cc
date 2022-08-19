/*
  Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "client/dump/compression_zlib_writer.h"
#include "template_utils.h"

#include <functional>

using namespace Mysql::Tools::Dump;

void Compression_zlib_writer::process_buffer(bool flush_stream) {
  do {
    m_compression_context.avail_out = Compression_zlib_writer::buffer_size;
    m_compression_context.next_out = (Bytef *)&m_buffer[0];

    int res =
        deflate(&m_compression_context, flush_stream ? Z_FINISH : Z_NO_FLUSH);

    if (res == Z_STREAM_ERROR) {
      this->pass_message(Mysql::Tools::Base::Message_data(
          0, "zlib compression failed",
          Mysql::Tools::Base::Message_type_error));
    }

    this->append_output(
        std::string(&m_buffer[0], Compression_zlib_writer::buffer_size -
                                      m_compression_context.avail_out));
  } while (m_compression_context.avail_in > 0);
}

void Compression_zlib_writer::append(const std::string &data_to_append) {
  std::lock_guard<std::mutex> lock(m_zlib_mutex);
  m_compression_context.avail_in = data_to_append.size();
  m_compression_context.next_in =
      pointer_cast<Bytef *>(const_cast<char *>(data_to_append.data()));
  this->process_buffer(false);
}

Compression_zlib_writer::~Compression_zlib_writer() {
  std::lock_guard<std::mutex> lock(m_zlib_mutex);
  this->process_buffer(true);
  deflateEnd(&m_compression_context);
}

Compression_zlib_writer::Compression_zlib_writer(
    std::function<bool(const Mysql::Tools::Base::Message_data &)>
        *message_handler,
    Simple_id_generator *object_id_generator, uint compression_level)
    : Abstract_output_writer_wrapper(message_handler, object_id_generator),
      m_compression_level(compression_level) {}

bool Compression_zlib_writer::init() {
  memset(&m_compression_context, 0, sizeof(m_compression_context));
  m_buffer.resize(Compression_zlib_writer::buffer_size);
  int ret = deflateInit(&m_compression_context, m_compression_level);
  if (ret != Z_OK) {
    this->pass_message(Mysql::Tools::Base::Message_data(
        0, "zlib compression initialization failed",
        Mysql::Tools::Base::Message_type_error));
    return true;
  }
  return false;
}
