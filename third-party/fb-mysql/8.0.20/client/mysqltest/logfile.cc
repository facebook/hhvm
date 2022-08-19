// Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

#include "client/mysqltest/logfile.h"

#include <cstring>
#include <iostream>
#include <string>

Logfile::Logfile() : m_file(nullptr), m_bytes_written(0) {
  std::memset(m_filename, 0, sizeof(m_filename));
}

bool Logfile::flush() {
  if (m_file && std::fflush(m_file)) {
    // Flush failed, thrown an error.
    std::cerr << "Failed to flush the contents to file '" << m_filename
              << "': " << std::strerror(errno) << ", errno: " << errno
              << std::endl;
    return true;
  }
  return false;
}

bool Logfile::open(const char *dirname, const char *filename, const char *ext) {
  if (!filename) {
    m_file = stdout;
  } else {
    fn_format(m_filename, filename, dirname, ext,
              *dirname ? MY_REPLACE_DIR | MY_REPLACE_EXT : MY_REPLACE_EXT);

    this->close();

    // Open a file, print an error message if the operation fails.
    if ((m_file = std::fopen(m_filename, "wb+")) == nullptr) {
      std::cerr << "Failed to open log file '" << m_filename
                << "': " << std::strerror(errno) << ", errno: " << errno
                << std::endl;
      return true;
    }
  }
  return false;
}

bool Logfile::write(const char *data, std::size_t length) {
  if (length > 0) {
    if (std::fwrite(data, 1, length, m_file) != length) {
      std::cerr << "Failed to write " << length << "bytes to '" << m_filename
                << "': " << std::strerror(errno) << ", errno: " << errno
                << std::endl;
      return true;
    }
    m_bytes_written += length;
  }
  return false;
}

void Logfile::close() {
  if (m_file) {
    if (m_file == stdout)
      std::fflush(m_file);
    else
      std::fclose(m_file);
  }
  m_file = nullptr;
}

void Logfile::show_tail(unsigned int lines) {
  if (!m_file || m_file == stdout || lines == 0) return;

  // Move to end of the file.
  if (std::fseek(m_file, 0, SEEK_END) != 0) {
    std::cerr << "fseek() failed: " << std::strerror(errno)
              << ", errno: " << errno << std::endl;
    return;
  }

  int position = std::ftell(m_file);
  unsigned int count = 0;

  while (position) {
    std::fseek(m_file, --position, SEEK_SET);
    // Read one character at a time, and stop reading when N number of
    // newline characters are found.
    if ((std::fgetc(m_file) == '\n') && (count++ == lines)) break;
  }

  std::cerr << std::endl
            << "The result from queries just before the failure was:"
            << std::endl;

  char buffer[256];
  std::size_t bytes;
  if (count <= lines) std::fseek(m_file, 0L, SEEK_SET);
  while ((bytes = std::fread(buffer, 1, sizeof(buffer), m_file)) > 0) {
    if (std::fwrite(buffer, 1, bytes, stderr) != bytes)
      std::cerr << "fwrite failed: " << std::strerror(errno)
                << ", errno: " << errno << std::endl;
  }

  std::fflush(stderr);
}
