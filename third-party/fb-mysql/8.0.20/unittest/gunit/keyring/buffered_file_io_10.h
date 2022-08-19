/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "my_inttypes.h"
#include "plugin/keyring/buffered_file_io.h"

namespace keyring {
class Buffered_file_io_10 : public Buffered_file_io {
 public:
  Buffered_file_io_10(ILogger *logger)
      : Buffered_file_io(logger), file_version("Keyring file version:1.0") {}
  bool flush_to_file(PSI_file_key *file_key, const std::string *filename,
                     const Digest *digest);
  size_t get_memory_needed_for_buffer();

 private:
  std::string file_version;
};

}  // namespace keyring
