/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/basic_istream.h"
#include <my_io.h>
#include <my_sys.h>
#include <mysql/psi/mysql_file.h>

IO_CACHE_istream::IO_CACHE_istream() {}

IO_CACHE_istream::~IO_CACHE_istream() { close(); }

bool IO_CACHE_istream::open(
#ifdef HAVE_PSI_INTERFACE
    PSI_file_key log_file_key MY_ATTRIBUTE((unused)),
    PSI_file_key log_cache_key,
#endif
    const char *file_name, myf flags MY_ATTRIBUTE((unused)),
    size_t cache_size) {
  File file = -1;

  file = mysql_file_open(log_file_key, file_name, O_RDONLY, MYF(MY_WME));
  if (file < 0) return true;

#ifdef HAVE_PSI_INTERFACE
  if (init_io_cache_ext(&m_io_cache, file, cache_size, READ_CACHE, 0, false,
                        flags, log_cache_key))
#else
  if (init_io_cache(&m_io_cache, file, cache_size, READ_CACHE, 0, false,
                    MYF(MY_WME | MY_DONT_CHECK_FILESIZE)))
#endif
  {
    mysql_file_close(file, MYF(0));
    return true;
  }
  return false;
}

void IO_CACHE_istream::close() {
  if (my_b_inited(&m_io_cache)) {
    end_io_cache(&m_io_cache);
    mysql_file_close(m_io_cache.file, MYF(MY_WME));
  }
}

my_off_t IO_CACHE_istream::length() { return my_b_filelength(&m_io_cache); }

ssize_t IO_CACHE_istream::read(unsigned char *buffer, size_t length) {
  DBUG_TRACE;
  if (my_b_read(&m_io_cache, buffer, length) ||
      DBUG_EVALUATE_IF("simulate_magic_header_io_failure", 1, 0))
    return m_io_cache.error;
  return static_cast<longlong>(length);
}

bool IO_CACHE_istream::seek(my_off_t offset) {
  DBUG_TRACE;
  bool res = false;
  my_b_seek(&m_io_cache, offset);
  DBUG_EXECUTE_IF("simulate_seek_failure", res = true;);
  return res;
}

Stdin_istream::Stdin_istream() {}

Stdin_istream::~Stdin_istream() { close(); }

bool Stdin_istream::open(std::string *errmsg) {
/* read from stdin */
/*
  Windows opens stdin in text mode by default. Certain characters
  such as CTRL-Z are interpeted as events and the read() method
  will stop. CTRL-Z is the EOF marker in Windows. to get past this
  you have to open stdin in binary mode. Setmode() is used to set
  stdin in binary mode. Errors on setting this mode result in
  halting the function and printing an error message to stderr.
*/
#if defined(_WIN32)
  if (_setmode(fileno(stdin), _O_BINARY) == -1) {
    *errmsg = "Could not set binary mode on stdin.";
    return true;
  }
#endif
  if (init_io_cache(
          &m_io_cache, my_fileno(stdin), 0, READ_CACHE, 0, false,
          MYF(MY_WME | MY_NABP | MY_DONT_CHECK_FILESIZE | MY_FULL_IO))) {
    *errmsg = "Failed to init IO cache.";
    return true;
  }
  return false;
}

void Stdin_istream::close() { end_io_cache(&m_io_cache); }

ssize_t Stdin_istream::read(unsigned char *buffer, size_t length) {
  if (my_b_read(&m_io_cache, buffer, length)) return m_io_cache.error;
  return static_cast<longlong>(length);
}

bool Stdin_istream::skip(my_off_t bytes) {
  /* Just refill the cache If all data in it should be skipped. */
  while (bytes > my_b_bytes_in_cache(&m_io_cache)) {
    bytes -= my_b_bytes_in_cache(&m_io_cache);
    if (my_b_fill(&m_io_cache) == 0) return m_io_cache.error == -1;
  }

  m_io_cache.read_pos += bytes;
  return false;
}
