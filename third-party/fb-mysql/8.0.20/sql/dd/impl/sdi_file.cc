/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/sdi_file.h"

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <memory>
#include <ostream>
#include <string>

#include "mysql/components/services/psi_file_bits.h"
#include "mysql/udf_registration_types.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "lex_string.h"
#include "m_ctype.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_file.h"  // mysql_file_create
#include "mysqld_error.h"
#include "sql/dd/impl/sdi.h"        // dd::Sdi_type
#include "sql/dd/impl/sdi_utils.h"  // dd::sdi_util::checked_return
#include "sql/dd/types/schema.h"    // dd::Schema
#include "sql/dd/types/table.h"     // dd::Table
#include "sql/key.h"
#include "sql/mysqld.h"  // is_secure_file_path
#include "sql/sql_class.h"
#include "sql/sql_const.h"  // CREATE_MODE
#include "sql/sql_table.h"  // build_table_filename

/**
  @file
  @ingroup sdi

  Storage and retrieval of SDIs to/from files. Default for SEs which do
  not have the ability to store SDIs in tablespaces. File storage is
  not transactional.
*/

using namespace dd::sdi_utils;

extern PSI_file_key key_file_sdi;
namespace {

bool write_sdi_file(const dd::String_type &fname, const dd::Sdi_type &sdi) {
  File sdif = mysql_file_create(key_file_sdi, fname.c_str(), CREATE_MODE,
                                O_WRONLY | O_TRUNC, MYF(MY_FAE));
  if (sdif < 0) {
    char errbuf[MYSYS_STRERROR_SIZE];
    my_error(ER_CANT_CREATE_FILE, MYF(0), fname.c_str(), my_errno(),
             my_strerror(errbuf, sizeof(errbuf), my_errno()));
    return checked_return(true);
  }

  size_t bw =
      mysql_file_write(sdif, reinterpret_cast<const uchar *>(sdi.c_str()),
                       sdi.length(), MYF(MY_FNABP));

  if (bw == MY_FILE_ERROR) {
#ifndef DBUG_OFF
    bool close_error =
#endif /* !DBUG_OFF */
        mysql_file_close(sdif, MYF(0));
    DBUG_ASSERT(close_error == false);
    return checked_return(true);
  }
  DBUG_ASSERT(bw == 0);
  return checked_return(mysql_file_close(sdif, MYF(MY_FAE)));
}

bool sdi_file_exists(const dd::String_type &fname, bool *res) {
#ifndef _WIN32

  if (my_access(fname.c_str(), F_OK) == 0) {
    *res = true;
    return false;
  }

#else /* _WIN32 */
  // my_access cannot be used to test for the absence of a file on Windows
  WIN32_FILE_ATTRIBUTE_DATA fileinfo;
  BOOL result =
      GetFileAttributesEx(fname.c_str(), GetFileExInfoStandard, &fileinfo);
  if (result) {
    *res = true;
    return false;
  }

  my_osmaperr(GetLastError());

#endif /* _WIN32 */

  if (errno == ENOENT) {
    *res = false;
    return false;
  }

  char errbuf[MYSYS_STRERROR_SIZE];
  my_error(ER_CANT_GET_STAT, MYF(0), fname.c_str(), errno,
           my_strerror(errbuf, sizeof(errbuf), errno));
  return checked_return(true);
}

int pathncmp(const LEX_CSTRING &a, const LEX_CSTRING &b, size_t n) {
  if (!lower_case_file_system) {
    return strncmp(a.str, b.str, n);
  }
  return files_charset_info->coll->strnncoll(
      files_charset_info, reinterpret_cast<const uchar *>(a.str), a.length,
      reinterpret_cast<const uchar *>(b.str), b.length, false);
}

struct Dir_pat_tuple {
  dd::String_type dir;
  dd::String_type pat;
  bool m_is_inside_datadir;
};

Dir_pat_tuple make_dir_pattern_tuple(const LEX_STRING &path,
                                     const LEX_CSTRING &schema_name) {
  char dirname[FN_REFLEN];
  size_t dirname_len = 0;
  // Return value same as dirname_len?
  dirname_part(dirname, path.str, &dirname_len);
  const dd::String_type fpat{path.str + dirname_len, path.length - dirname_len};

  dd::String_type data_dir{mysql_real_data_home_ptr};
  if (test_if_hard_path(path.str)) {
    bool is_in_datadir = false;
    if (dirname_len >= data_dir.length()) {
      const char *dirname_begin = dirname;
      // const char *dirname_end= dirname+dirname_len;
      is_in_datadir = (pathncmp({dirname_begin, dirname_len},
                                {data_dir.c_str(), data_dir.length()},
                                data_dir.length()) == 0);
    }
    return {dd::String_type{dirname, dirname_len}, std::move(fpat),
            is_in_datadir};
  }

  if (dirname_len == 0) {
    data_dir.append(schema_name.str, schema_name.length);
    data_dir.push_back(FN_LIBCHAR);
  } else {
    data_dir.append(dirname, dirname_len);
  }

  return {std::move(data_dir), std::move(fpat), true};
}

bool expand_sdi_pattern(const Dir_pat_tuple &dpt,
                        dd::sdi_file::Paths_type *paths) {
  const char *dir_beg = dpt.dir.c_str();

  if (!is_secure_file_path(dir_beg)) {
    dd::String_type x = "--secure-file-priv='";
    x.append(opt_secure_file_priv);
    x.append("'");
    /* Read only allowed from within dir specified by secure_file_priv */
    my_error(ER_OPTION_PREVENTS_STATEMENT, MYF(0), x.c_str(), "");
    return true;
  }

  MY_DIR *dir = my_dir(dir_beg, MYF(MY_DONT_SORT));
  if (dir == nullptr) {
    char errbuf[MYSYS_STRERROR_SIZE];
    my_error(ER_CANT_READ_DIR, MYF(0), dir_beg, my_errno,
             my_strerror(errbuf, sizeof(errbuf), my_errno()));
    return true;
  }

  auto dirender = [](MY_DIR *d) { my_dirend(d); };
  std::unique_ptr<MY_DIR, decltype(dirender)> guard(dir, dirender);

  const char *pat_beg = dpt.pat.c_str();
  const char *pat_end = dpt.pat.c_str() + dpt.pat.length();

  size_t path_count = paths->size();
  for (uint i = 0; i < dir->number_off_files; ++i) {
    bool match = false;
    if (!lower_case_file_system)
      match = (my_wildcmp_mb_bin(
                   files_charset_info, dir->dir_entry[i].name,
                   dir->dir_entry[i].name + strlen(dir->dir_entry[i].name),
                   pat_beg, pat_end, '\\', '?', '*') == 0);
    else
      match =
          (my_wildcmp(files_charset_info, dir->dir_entry[i].name,
                      dir->dir_entry[i].name + strlen(dir->dir_entry[i].name),
                      pat_beg, pat_end, '\\', '?', '*') == 0);
    if (match) {
      size_t len = strlen(dir->dir_entry[i].name);
      const char *tail = dir->dir_entry[i].name + len - 4;
      if (len < 4 || my_strcasecmp(files_charset_info, tail, ".sdi") != 0) {
        my_error(ER_WRONG_FILE_NAME, MYF(0), dir->dir_entry[i].name);
        return true;
      }

      char sdi_pathname[FN_REFLEN];
      fn_format(sdi_pathname, dir->dir_entry[i].name, dir_beg, "",
                MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH));
      paths->emplace_back(dd::String_type(sdi_pathname),
                          dpt.m_is_inside_datadir);
    }
  }
  if (path_count == paths->size()) {
    my_error(ER_IMP_NO_FILES_MATCHED, MYF(0), pat_beg);
    return true;
  }
  return false;
}

}  // namespace

namespace dd {
namespace sdi_file {

String_type sdi_filename(Object_id id, const String_type &entity_name,
                         const String_type &schema) {
  typedef String_type::const_iterator CHARIT;
  const CHARIT begin = entity_name.begin();
  const CHARIT end = entity_name.end();
  CHARIT i = begin;
  size_t count = 0;

  while (i != end && count < dd::sdi_file::FILENAME_PREFIX_CHARS) {
    size_t charlen = my_mbcharlen(system_charset_info, static_cast<uchar>(*i));
    DBUG_ASSERT(charlen > 0);
    i += charlen;
    ++count;
  }

  Stringstream_type fnamestr;
  fnamestr << String_type(begin, i) << "_" << id;

  char path[FN_REFLEN + 1];
  bool was_truncated = false;
  build_table_filename(path, sizeof(path) - 1, schema.c_str(),
                       fnamestr.str().c_str(), EXT.c_str(), 0, &was_truncated);
  DBUG_ASSERT(!was_truncated);

  return String_type(path);
}

bool store_tbl_sdi(const dd::Sdi_type &sdi, const dd::Table &table,
                   const dd::Schema &schema) {
  return checked_return(write_sdi_file(
      sdi_filename(table.id(), table.name(), schema.name()), sdi));
}

bool remove(const String_type &fname) {
  return checked_return(
      mysql_file_delete(key_file_sdi, fname.c_str(), MYF(MY_FAE)));
}

static bool remove_sdi_file_if_exists(const String_type &fname) {
  bool file_exists = false;
  if (sdi_file_exists(fname, &file_exists)) {
    return checked_return(true);
  }

  if (!file_exists) {
    return false;
  }

  return checked_return(remove(fname));
}

bool drop_tbl_sdi(const dd::Table &table, const dd::Schema &schema) {
  String_type sdi_fname = sdi_filename(table.id(), table.name(), schema.name());
  return checked_return(remove_sdi_file_if_exists(sdi_fname));
}

bool load(THD *, const dd::String_type &fname, dd::String_type *buf) {
  File sdi_fd = mysql_file_open(key_file_sdi, fname.c_str(), O_RDONLY,
                                MYF(MY_FAE | MY_WME));
  if (sdi_fd < 0) {
    return dd::sdi_utils::checked_return(true);
  }

  auto closer = [](File *f) {
#ifndef DBUG_OFF
    bool ret =
#endif /* !DBUG_OFF */
        mysql_file_close(*f, MYF(MY_FAE | MY_WME));
    DBUG_ASSERT(ret == false);
  };
  std::unique_ptr<File, decltype(closer)> guard(&sdi_fd, closer);

  MY_STAT mystat;
  if (mysql_file_fstat(sdi_fd, &mystat)) {
    return true;
  }

  if (mystat.st_size == 0) return false;
  buf->resize(static_cast<size_t>(mystat.st_size));
  uchar *sdi_buf = reinterpret_cast<uchar *>(&buf->front());

  if (mysql_file_read(sdi_fd, sdi_buf, buf->size(),
                      MYF(MY_FAE | MY_WME | MY_NABP))) {
    return true;
  }
  return false;
}

bool expand_pattern(THD *thd, const LEX_STRING &pattern, Paths_type *paths) {
  auto dpt = make_dir_pattern_tuple(pattern, thd->db());
  if (expand_sdi_pattern(dpt, paths)) {
    return true;
  }
  return false;
}

template <typename CLOS>
bool with_str_error(CLOS &&clos) {
  char errbuf[MYSYS_STRERROR_SIZE];
  return clos(my_strerror(errbuf, sizeof(errbuf), my_errno()));
}

bool check_data_files_exist(const dd::String_type &schema_name,
                            const dd::String_type &table_name) {
  char path[FN_REFLEN + 1];
  bool was_truncated = false;
  size_t plen =
      build_table_filename(path, sizeof(path) - 1, schema_name.c_str(),
                           table_name.c_str(), ".MYD", 0, &was_truncated);

  MY_STAT sa;
  if (nullptr == my_stat(path, &sa, MYF(0))) {
    with_str_error([&](const char *strerr) {
      my_error(ER_FILE_NOT_FOUND, MYF(0), path, my_errno(), strerr);
      return false;
    });
    return true;
  }

  path[plen - 3] = 0;
  strcat(path, "MYI");

  if (nullptr == my_stat(path, &sa, MYF(0))) {
    with_str_error([&](const char *strerr) {
      my_error(ER_FILE_NOT_FOUND, MYF(0), path, my_errno(), strerr);
      return false;
    });
    return true;
  }
  return false;
}
}  // namespace sdi_file
}  // namespace dd
