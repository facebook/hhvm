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

#ifndef COMPRESSION_ZLIB_WRITER_INCLUDED
#define COMPRESSION_ZLIB_WRITER_INCLUDED

#include <string.h>
#include <sys/types.h>
#include <zlib.h>
#include <functional>
#include <mutex>

#include "client/dump/abstract_output_writer_wrapper.h"
#include "client/dump/i_output_writer.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Wrapper to another Output Writer, compresses formatted data stream with zlib.
 */
class Compression_zlib_writer : public I_output_writer,
                                public Abstract_output_writer_wrapper {
 public:
  Compression_zlib_writer(
      std::function<bool(const Mysql::Tools::Base::Message_data &)>
          *message_handler,
      Simple_id_generator *object_id_generator, uint compression_level);

  ~Compression_zlib_writer();

  bool init();
  void append(const std::string &data_to_append);

  // Fix "inherits ... via dominance" warnings
  void register_progress_watcher(I_progress_watcher *new_progress_watcher) {
    Abstract_chain_element::register_progress_watcher(new_progress_watcher);
  }

  // Fix "inherits ... via dominance" warnings
  uint64 get_id() const { return Abstract_chain_element::get_id(); }

 protected:
  // Fix "inherits ... via dominance" warnings
  void item_completion_in_child_callback(Item_processing_data *item_processed) {
    Abstract_chain_element::item_completion_in_child_callback(item_processed);
  }

 private:
  void process_buffer(bool flush_stream);

  std::mutex m_zlib_mutex;
  z_stream m_compression_context;
  uint m_compression_level;
  std::vector<char> m_buffer;

  const static int buffer_size = 128 * 1024;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
