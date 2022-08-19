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

#include "unittest/gunit/keyring/buffered_file_io_10.h"

#include <fcntl.h>
#include <mysql/psi/mysql_file.h>

#include "my_io.h"

namespace keyring {
bool Buffered_file_io_10::flush_to_file(
    PSI_file_key *file_key MY_ATTRIBUTE((unused)), const std::string *filename,
    const Digest *) {
  File file;
  bool was_error = true;
  file = mysql_file_open(*file_key, filename->c_str(),
                         O_TRUNC | O_WRONLY | O_CREAT, MYF(0));
  if (file >= 0 &&
      mysql_file_write(
          file, reinterpret_cast<const uchar *>(file_version.c_str()),
          file_version.length(), MYF(0)) == file_version.length() &&
      mysql_file_write(file, buffer.data, buffer.size, MYF(0)) == buffer.size &&
      mysql_file_write(
          file, reinterpret_cast<const uchar *>(Checker::eofTAG.c_str()),
          Checker::eofTAG.length(), MYF(0)) == Checker::eofTAG.length() &&
      mysql_file_close(file, MYF(0)) >= 0) {
    was_error = false;
  }
  buffer.free();
  return was_error;
}

size_t Buffered_file_io_10::get_memory_needed_for_buffer() {
  return memory_needed_for_buffer;
}
}  // namespace keyring
