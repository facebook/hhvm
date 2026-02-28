/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/charset.cc
*/

#include "my_config.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <mutex>
#include <unordered_map>

#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_sys.h"
#include "my_thread.h"
#include "my_xml.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"
#include "sql_chars.h"

/*
  The code below implements this functionality:

    - Initializing charset related structures
    - Loading dynamic charsets
    - Searching for a proper CHARSET_INFO
      using charset name, collation name or collation ID
    - Setting server default character set
*/

extern CHARSET_INFO my_charset_ucs2_unicode_ci;
extern CHARSET_INFO my_charset_utf8mb4_unicode_ci;
extern CHARSET_INFO my_charset_utf16_unicode_ci;
extern CHARSET_INFO my_charset_utf32_unicode_ci;
extern CHARSET_INFO my_charset_cp932_japanese_ci;

bool my_charset_same(const CHARSET_INFO *cs1, const CHARSET_INFO *cs2) {
  return ((cs1 == cs2) || !strcmp(cs1->csname, cs2->csname));
}

std::unordered_map<std::string, int> *coll_name_num_map = nullptr;
std::unordered_map<std::string, int> *cs_name_pri_num_map = nullptr;
std::unordered_map<std::string, int> *cs_name_bin_num_map = nullptr;

#define MY_CS_BUFFER_SIZE (MY_CS_NAME_SIZE * 8)

static void map_coll_name_to_number(const char *name, int num) {
  char lower_case_name[MY_CS_BUFFER_SIZE] = {0};
  size_t len = std::min(strlen(name), sizeof(lower_case_name) - 2);
  memcpy(lower_case_name, name, len);
  lower_case_name[len] = '\0';
  my_casedn_str(&my_charset_latin1, lower_case_name);

  DBUG_ASSERT(coll_name_num_map != nullptr);
  (*coll_name_num_map)[lower_case_name] = num;
}

static void map_cs_name_to_number(const char *name, int num, int state) {
  char lower_case_name[MY_CS_BUFFER_SIZE] = {0};
  size_t len = std::min(strlen(name), sizeof(lower_case_name) - 2);
  memcpy(lower_case_name, name, len);
  lower_case_name[len] = '\0';
  my_casedn_str(&my_charset_latin1, lower_case_name);

  DBUG_ASSERT(cs_name_pri_num_map != nullptr && cs_name_bin_num_map != nullptr);
  if ((state & MY_CS_PRIMARY)) (*cs_name_pri_num_map)[lower_case_name] = num;
  if ((state & MY_CS_BINSORT)) (*cs_name_bin_num_map)[lower_case_name] = num;
}

static uint get_collation_number_internal(const char *name) {
  char lower_case_name[MY_CS_BUFFER_SIZE] = {0};
  size_t len = std::min(strlen(name), sizeof(lower_case_name) - 2);
  memcpy(lower_case_name, name, len);
  lower_case_name[len] = '\0';
  my_casedn_str(&my_charset_latin1, lower_case_name);

  DBUG_ASSERT(coll_name_num_map != nullptr);
  auto name_num_map_it = coll_name_num_map->find(lower_case_name);
  if (name_num_map_it != coll_name_num_map->end())
    return name_num_map_it->second;
  return 0;
}

static void simple_cs_init_functions(CHARSET_INFO *cs) {
  if (cs->state & MY_CS_BINSORT)
    cs->coll = &my_collation_8bit_bin_handler;
  else
    cs->coll = &my_collation_8bit_simple_ci_handler;

  cs->cset = &my_charset_8bit_handler;
}

static bool cs_copy_data(CHARSET_INFO *to, CHARSET_INFO *from) {
  to->number = from->number ? from->number : to->number;

  if (from->csname) {
    to->csname = my_once_strdup(from->csname, MYF(MY_WME));
    if (to->csname == nullptr) return true;
  }

  if (from->name) {
    to->name = my_once_strdup(from->name, MYF(MY_WME));
    if (to->name == nullptr) return true;
  }

  if (from->comment) {
    to->comment = my_once_strdup(from->comment, MYF(MY_WME));
    if (to->comment == nullptr) return true;
  }

  if (from->ctype) {
    to->ctype = static_cast<uchar *>(
        my_once_memdup(from->ctype, MY_CS_CTYPE_TABLE_SIZE, MYF(MY_WME)));
    if (to->ctype == nullptr) return true;
    if (init_state_maps(to)) return true;
  }

  if (from->to_lower) {
    to->to_lower = static_cast<uchar *>(
        my_once_memdup(from->to_lower, MY_CS_TO_LOWER_TABLE_SIZE, MYF(MY_WME)));
    if (to->to_lower == nullptr) return true;
  }

  if (from->to_upper) {
    to->to_upper = static_cast<uchar *>(
        my_once_memdup(from->to_upper, MY_CS_TO_UPPER_TABLE_SIZE, MYF(MY_WME)));
    if (to->to_upper == nullptr) return true;
  }

  if (from->sort_order) {
    to->sort_order = static_cast<uchar *>(my_once_memdup(
        from->sort_order, MY_CS_SORT_ORDER_TABLE_SIZE, MYF(MY_WME)));
    if (to->sort_order == nullptr) return true;
  }

  if (from->tab_to_uni) {
    size_t sz = MY_CS_TO_UNI_TABLE_SIZE * sizeof(uint16);
    to->tab_to_uni = static_cast<uint16 *>(
        my_once_memdup(from->tab_to_uni, sz, MYF(MY_WME)));
    if (to->tab_to_uni == nullptr) return true;
  }

  if (from->tailoring) {
    to->tailoring = my_once_strdup(from->tailoring, MYF(MY_WME));
    if (to->tailoring == nullptr) return true;
  }

  return false;
}

static bool simple_cs_is_full(CHARSET_INFO *cs) {
  return ((cs->csname && cs->tab_to_uni && cs->ctype && cs->to_upper &&
           cs->to_lower) &&
          (cs->number && cs->name &&
           (cs->sort_order || (cs->state & MY_CS_BINSORT))));
}

static void copy_uca_collation(CHARSET_INFO *to, CHARSET_INFO *from) {
  to->cset = from->cset;
  to->coll = from->coll;
  to->strxfrm_multiply = from->strxfrm_multiply;
  to->min_sort_char = from->min_sort_char;
  to->max_sort_char = from->max_sort_char;
  to->mbminlen = from->mbminlen;
  to->mbmaxlen = from->mbmaxlen;
  to->caseup_multiply = from->caseup_multiply;
  to->casedn_multiply = from->casedn_multiply;
  to->state |= MY_CS_AVAILABLE | MY_CS_LOADED | MY_CS_STRNXFRM | MY_CS_UNICODE;
}

static int add_collation(CHARSET_INFO *cs) {
  // Disallow overwriting internal character sets.
  if (cs->name == nullptr || get_collation_number_internal(cs->name) != 0) {
    return MY_XML_OK;  // Just ignore it.
  }

  if (cs->number != 0 && cs->number < array_elements(all_charsets)) {
    if (!all_charsets[cs->number]) {
      if (!(all_charsets[cs->number] =
                (CHARSET_INFO *)my_once_alloc(sizeof(CHARSET_INFO), MYF(0))))
        return MY_XML_ERROR;
      memset(all_charsets[cs->number], 0, sizeof(CHARSET_INFO));
    }

    if (cs->primary_number == cs->number) cs->state |= MY_CS_PRIMARY;

    if (cs->binary_number == cs->number) cs->state |= MY_CS_BINSORT;

    all_charsets[cs->number]->state |= cs->state;
    map_coll_name_to_number(cs->name, cs->number);
    map_cs_name_to_number(cs->csname, cs->number, cs->state);

    if (!(all_charsets[cs->number]->state & MY_CS_COMPILED)) {
      CHARSET_INFO *newcs = all_charsets[cs->number];
      if (cs_copy_data(all_charsets[cs->number], cs)) return MY_XML_ERROR;

      newcs->caseup_multiply = newcs->casedn_multiply = 1;
      newcs->levels_for_compare = 1;

      if (!strcmp(cs->csname, "ucs2")) {
        copy_uca_collation(newcs, &my_charset_ucs2_unicode_ci);
        newcs->state |= MY_CS_AVAILABLE | MY_CS_LOADED | MY_CS_NONASCII;
      } else if (!strcmp(cs->csname, "utf8") ||
                 !strcmp(cs->csname, "utf8mb3")) {
        copy_uca_collation(newcs, &my_charset_utf8_unicode_ci);
        newcs->ctype = my_charset_utf8_unicode_ci.ctype;
        if (init_state_maps(newcs)) return MY_XML_ERROR;
      } else if (!strcmp(cs->csname, "utf8mb4")) {
        copy_uca_collation(newcs, &my_charset_utf8mb4_unicode_ci);
        newcs->ctype = my_charset_utf8mb4_unicode_ci.ctype;
        newcs->state |= MY_CS_AVAILABLE | MY_CS_LOADED;
      } else if (!strcmp(cs->csname, "utf16")) {
        copy_uca_collation(newcs, &my_charset_utf16_unicode_ci);
        newcs->state |= MY_CS_AVAILABLE | MY_CS_LOADED | MY_CS_NONASCII;
      } else if (!strcmp(cs->csname, "utf32")) {
        copy_uca_collation(newcs, &my_charset_utf32_unicode_ci);
        newcs->state |= MY_CS_AVAILABLE | MY_CS_LOADED | MY_CS_NONASCII;
      } else {
        const uchar *sort_order = all_charsets[cs->number]->sort_order;
        simple_cs_init_functions(all_charsets[cs->number]);
        newcs->mbminlen = 1;
        newcs->mbmaxlen = 1;
        if (simple_cs_is_full(all_charsets[cs->number])) {
          all_charsets[cs->number]->state |= MY_CS_LOADED;
        }
        all_charsets[cs->number]->state |= MY_CS_AVAILABLE;

        /*
          Check if case sensitive sort order: A < a < B.
          We need MY_CS_FLAG for regex library, and for
          case sensitivity flag for 5.0 client protocol,
          to support isCaseSensitive() method in JDBC driver
        */
        if (sort_order &&
            sort_order[static_cast<int>('A')] <
                sort_order[static_cast<int>('a')] &&
            sort_order[static_cast<int>('a')] <
                sort_order[static_cast<int>('B')])
          all_charsets[cs->number]->state |= MY_CS_CSSORT;

        if (my_charset_is_8bit_pure_ascii(all_charsets[cs->number]))
          all_charsets[cs->number]->state |= MY_CS_PUREASCII;
        if (!my_charset_is_ascii_compatible(cs))
          all_charsets[cs->number]->state |= MY_CS_NONASCII;
      }
    } else {
      /*
        We need the below to make get_charset_name()
        and get_charset_number() working even if a
        character set has not been really incompiled.
        The above functions are used for example
        in error message compiler utilities/comp_err.cc.
        If a character set was compiled, this information
        will get lost and overwritten in add_compiled_collation().
      */
      CHARSET_INFO *dst = all_charsets[cs->number];
      dst->number = cs->number;
      if (cs->comment)
        if (!(dst->comment = my_once_strdup(cs->comment, MYF(MY_WME))))
          return MY_XML_ERROR;
      if (cs->csname)
        if (!(dst->csname = my_once_strdup(cs->csname, MYF(MY_WME))))
          return MY_XML_ERROR;
      if (cs->name)
        if (!(dst->name = my_once_strdup(cs->name, MYF(MY_WME))))
          return MY_XML_ERROR;
    }
    cs->number = 0;
    cs->primary_number = 0;
    cs->binary_number = 0;
    cs->name = nullptr;
    cs->state = 0;
    cs->sort_order = nullptr;
    cs->state = 0;
  }
  return MY_XML_OK;
}

/**
  Report character set initialization errors and warnings.
  Be silent by default: no warnings on the client side.
*/
static void default_reporter(enum loglevel level MY_ATTRIBUTE((unused)),
                             uint ecode MY_ATTRIBUTE((unused)), ...) {}
my_error_reporter my_charset_error_reporter = default_reporter;

/**
  Wrappers for memory functions my_malloc (and friends)
  with C-compatbile API without extra "myf" argument.
*/
static void *my_once_alloc_c(size_t size) {
  return my_once_alloc(size, MYF(MY_WME));
}

static void *my_malloc_c(size_t size) {
  return my_malloc(key_memory_charset_loader, size, MYF(MY_WME));
}

static void *my_realloc_c(void *old, size_t size) {
  return my_realloc(key_memory_charset_loader, old, size, MYF(MY_WME));
}

static void my_free_c(void *ptr) { my_free(ptr); }

/**
  Initialize character set loader to use mysys memory management functions.
  @param loader  Loader to initialize
*/
void my_charset_loader_init_mysys(MY_CHARSET_LOADER *loader) {
  loader->errcode = 0;
  loader->errarg[0] = '\0';
  loader->once_alloc = my_once_alloc_c;
  loader->mem_malloc = my_malloc_c;
  loader->mem_realloc = my_realloc_c;
  loader->mem_free = my_free_c;
  loader->reporter = my_charset_error_reporter;
  loader->add_collation = add_collation;
}

#define MY_MAX_ALLOWED_BUF 1024 * 1024
#define MY_CHARSET_INDEX "Index.xml"

const char *charsets_dir = nullptr;

static bool my_read_charset_file(MY_CHARSET_LOADER *loader,
                                 const char *filename, myf myflags) {
  uchar *buf;
  int fd;
  size_t len, tmp_len;
  MY_STAT stat_info;

  if (!my_stat(filename, &stat_info, MYF(myflags)) ||
      ((len = (uint)stat_info.st_size) > MY_MAX_ALLOWED_BUF) ||
      !(buf = (uchar *)my_malloc(key_memory_charset_file, len, myflags)))
    return true;

  if ((fd = mysql_file_open(key_file_charset, filename, O_RDONLY, myflags)) < 0)
    goto error;
  tmp_len = mysql_file_read(fd, buf, len, myflags);
  mysql_file_close(fd, myflags);
  if (tmp_len != len) goto error;

  if (my_parse_charset_xml(loader, (char *)buf, len)) {
    my_printf_error(EE_UNKNOWN_CHARSET, "Error while parsing '%s': %s\n",
                    MYF(0), filename, loader->errarg);
    goto error;
  }

  my_free(buf);
  return false;

error:
  my_free(buf);
  return true;
}

char *get_charsets_dir(char *buf) {
  const char *sharedir = SHAREDIR;
  char *res;
  DBUG_TRACE;

  if (charsets_dir != nullptr)
    strmake(buf, charsets_dir, FN_REFLEN - 1);
  else {
    if (test_if_hard_path(sharedir) ||
        is_prefix(sharedir, DEFAULT_CHARSET_HOME))
      strxmov(buf, sharedir, "/", CHARSET_DIR, NullS);
    else
      strxmov(buf, DEFAULT_CHARSET_HOME, "/", sharedir, "/", CHARSET_DIR,
              NullS);
  }
  res = convert_dirname(buf, buf, NullS);
  DBUG_PRINT("info", ("charsets dir: '%s'", buf));
  return res;
}

CHARSET_INFO *all_charsets[MY_ALL_CHARSETS_SIZE] = {nullptr};
CHARSET_INFO *default_charset_info = &my_charset_latin1;

void add_compiled_collation(CHARSET_INFO *cs) {
  DBUG_ASSERT(cs->number < array_elements(all_charsets));
  all_charsets[cs->number] = cs;
  map_coll_name_to_number(cs->name, cs->number);
  map_cs_name_to_number(cs->csname, cs->number, cs->state);
  cs->state |= MY_CS_AVAILABLE;
}

static std::once_flag charsets_initialized;

static void init_available_charsets(void) {
  char fname[FN_REFLEN + sizeof(MY_CHARSET_INDEX)];
  MY_CHARSET_LOADER loader;

  memset(&all_charsets, 0, sizeof(all_charsets));

  DBUG_ASSERT(coll_name_num_map == nullptr && cs_name_pri_num_map == nullptr &&
              cs_name_bin_num_map == nullptr);
  coll_name_num_map = new std::unordered_map<std::string, int>(0);
  cs_name_pri_num_map = new std::unordered_map<std::string, int>(0);
  cs_name_bin_num_map = new std::unordered_map<std::string, int>(0);
  init_compiled_charsets(MYF(0));

  /* Copy compiled charsets */

  my_charset_loader_init_mysys(&loader);
  my_stpcpy(get_charsets_dir(fname), MY_CHARSET_INDEX);
  my_read_charset_file(&loader, fname, MYF(0));
}

static const char *get_collation_name_alias(const char *name, char *buf,
                                            size_t bufsize) {
  if (!native_strncasecmp(name, "utf8mb3_", 8)) {
    snprintf(buf, bufsize, "utf8_%s", name + 8);
    return buf;
  }
  /* CLDR's tailoring data for Norwegian is same as Danish */
  if (!native_strncasecmp(name, "utf8mb4_no_0900_", 16)) {
    snprintf(buf, bufsize, "utf8mb4_da_0900_%s", name + 16);
    return buf;
  }
  return nullptr;
}

uint get_collation_number(const char *name) {
  uint id;
  char alias[64];
  std::call_once(charsets_initialized, init_available_charsets);
  if ((id = get_collation_number_internal(name))) return id;
  if ((name = get_collation_name_alias(name, alias, sizeof(alias))))
    return get_collation_number_internal(name);
  return 0;
}

static uint get_charset_number_internal(const char *charset_name,
                                        uint cs_flags) {
  char lower_case_name[MY_CS_BUFFER_SIZE] = {0};
  size_t len = std::min(strlen(charset_name), sizeof(lower_case_name) - 2);
  memcpy(lower_case_name, charset_name, len);
  lower_case_name[len] = '\0';
  my_casedn_str(&my_charset_latin1, lower_case_name);
  /*
    So far, all our calls to get the collation number by its charset name
    and flags is to get the PRIMARY / BIN collation of this charset.

    This function might be called concurrently. C++ guarantees this read-only
    access to STL container is thread-safe.
  */
  DBUG_ASSERT(cs_name_pri_num_map != nullptr && cs_name_bin_num_map != nullptr);
  if ((cs_flags & MY_CS_PRIMARY)) {
    auto name_num_map_it = cs_name_pri_num_map->find(lower_case_name);
    if (name_num_map_it != cs_name_pri_num_map->end()) {
      return name_num_map_it->second;
    } else {
      return 0;
    }
  }
  if ((cs_flags & MY_CS_BINSORT)) {
    auto name_num_map_it = cs_name_bin_num_map->find(lower_case_name);
    if (name_num_map_it != cs_name_bin_num_map->end()) {
      return name_num_map_it->second;
    } else {
      return 0;
    }
  }

  DBUG_ASSERT(false);
  return 0;
}

static const char *get_charset_name_alias(const char *name) {
  if (!my_strcasecmp(&my_charset_latin1, name, "utf8mb3")) return "utf8";
  return nullptr;
}

uint get_charset_number(const char *charset_name, uint cs_flags) {
  uint id;
  std::call_once(charsets_initialized, init_available_charsets);
  if ((id = get_charset_number_internal(charset_name, cs_flags))) return id;
  if ((charset_name = get_charset_name_alias(charset_name)))
    return get_charset_number_internal(charset_name, cs_flags);
  return 0;
}

const char *get_charset_name(uint charset_number) {
  std::call_once(charsets_initialized, init_available_charsets);

  if (charset_number < array_elements(all_charsets)) {
    CHARSET_INFO *cs = all_charsets[charset_number];

    if (cs && (cs->number == charset_number) && cs->name) return cs->name;
  }

  return "?"; /* this mimics find_type() */
}

static CHARSET_INFO *get_internal_charset(MY_CHARSET_LOADER *loader_arg,
                                          uint cs_number, myf flags) {
  char buf[FN_REFLEN];
  CHARSET_INFO *cs;

  DBUG_ASSERT(cs_number < array_elements(all_charsets));

  if ((cs = all_charsets[cs_number])) {
    if (cs->state & MY_CS_READY) /* if CS is already initialized */
      return cs;

    /*
      To make things thread safe we are not allowing other threads to interfere
      while we may changing the cs_info_table
    */
    mysql_mutex_lock(&THR_LOCK_charset);

    if (!(cs->state &
          (MY_CS_COMPILED | MY_CS_LOADED))) /* if CS is not in memory */
    {
      MY_CHARSET_LOADER loader;
      strxmov(get_charsets_dir(buf), cs->csname, ".xml", NullS);
      my_charset_loader_init_mysys(&loader);
      my_read_charset_file(&loader, buf, flags);
    }

    if (cs->state & MY_CS_AVAILABLE) {
      if (!(cs->state & MY_CS_READY)) {
        if ((cs->cset->init && cs->cset->init(cs, loader_arg)) ||
            (cs->coll->init && cs->coll->init(cs, loader_arg))) {
          cs = nullptr;
        } else
          cs->state |= MY_CS_READY;
      }
    } else
      cs = nullptr;

    mysql_mutex_unlock(&THR_LOCK_charset);
  }
  return cs;
}

CHARSET_INFO *get_charset(uint cs_number, myf flags) {
  CHARSET_INFO *cs;
  MY_CHARSET_LOADER loader;

  if (cs_number == default_charset_info->number) return default_charset_info;

  std::call_once(charsets_initialized, init_available_charsets);

  if (cs_number >= array_elements(all_charsets)) return nullptr;

  my_charset_loader_init_mysys(&loader);
  cs = get_internal_charset(&loader, cs_number, flags);

  if (!cs && (flags & MY_WME)) {
    char index_file[FN_REFLEN + sizeof(MY_CHARSET_INDEX)], cs_string[23];
    my_stpcpy(get_charsets_dir(index_file), MY_CHARSET_INDEX);
    cs_string[0] = '#';
    longlong10_to_str(cs_number, cs_string + 1, 10);
    my_error(EE_UNKNOWN_CHARSET, MYF(0), cs_string, index_file);
  }
  return cs;
}

/**
  Find collation by name: extended version of get_charset_by_name()
  to return error messages to the caller.
  @param   loader  Character set loader
  @param   name    Collation name
  @param   flags   Flags
  @return          NULL on error, pointer to collation on success
*/

CHARSET_INFO *my_collation_get_by_name(MY_CHARSET_LOADER *loader,
                                       const char *name, myf flags) {
  uint cs_number;
  CHARSET_INFO *cs;
  std::call_once(charsets_initialized, init_available_charsets);

  cs_number = get_collation_number(name);
  my_charset_loader_init_mysys(loader);
  cs = cs_number ? get_internal_charset(loader, cs_number, flags) : nullptr;

  if (!cs && (flags & MY_WME)) {
    char index_file[FN_REFLEN + sizeof(MY_CHARSET_INDEX)];
    my_stpcpy(get_charsets_dir(index_file), MY_CHARSET_INDEX);
    my_error(EE_UNKNOWN_COLLATION, MYF(0), name, index_file);
  }
  return cs;
}

CHARSET_INFO *get_charset_by_name(const char *cs_name, myf flags) {
  MY_CHARSET_LOADER loader;
  my_charset_loader_init_mysys(&loader);
  return my_collation_get_by_name(&loader, cs_name, flags);
}

/**
  Find character set by name: extended version of get_charset_by_csname()
  to return error messages to the caller.
  @param   loader   Character set loader
  @param   cs_name  Collation name
  @param   cs_flags Character set flags (e.g. default or binary collation)
  @param   flags    Flags
  @return           NULL on error, pointer to collation on success
*/
CHARSET_INFO *my_charset_get_by_name(MY_CHARSET_LOADER *loader,
                                     const char *cs_name, uint cs_flags,
                                     myf flags) {
  uint cs_number;
  CHARSET_INFO *cs;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("name: '%s'", cs_name));

  std::call_once(charsets_initialized, init_available_charsets);

  cs_number = get_charset_number(cs_name, cs_flags);
  cs = cs_number ? get_internal_charset(loader, cs_number, flags) : nullptr;

  if (!cs && (flags & MY_WME)) {
    char index_file[FN_REFLEN + sizeof(MY_CHARSET_INDEX)];
    my_stpcpy(get_charsets_dir(index_file), MY_CHARSET_INDEX);
    my_error(EE_UNKNOWN_CHARSET, MYF(0), cs_name, index_file);
  }

  return cs;
}

CHARSET_INFO *get_charset_by_csname(const char *cs_name, uint cs_flags,
                                    myf flags) {
  MY_CHARSET_LOADER loader;
  my_charset_loader_init_mysys(&loader);
  return my_charset_get_by_name(&loader, cs_name, cs_flags, flags);
}

/**
  Resolve character set by the character set name (utf8, latin1, ...).

  The function tries to resolve character set by the specified name. If
  there is character set with the given name, it is assigned to the "cs"
  parameter and false is returned. If there is no such character set,
  "default_cs" is assigned to the "cs" and true is returned.

  @param[in] cs_name    Character set name.
  @param[in] default_cs Default character set.
  @param[out] cs        Variable to store character set.

  @return false if character set was resolved successfully; true if there
  is no character set with given name.
*/

bool resolve_charset(const char *cs_name, const CHARSET_INFO *default_cs,
                     const CHARSET_INFO **cs) {
  *cs = get_charset_by_csname(cs_name, MY_CS_PRIMARY, MYF(0));

  if (*cs == nullptr) {
    *cs = default_cs;
    return true;
  }

  return false;
}

/**
  Resolve collation by the collation name (utf8_general_ci, ...).

  The function tries to resolve collation by the specified name. If there
  is collation with the given name, it is assigned to the "cl" parameter
  and false is returned. If there is no such collation, "default_cl" is
  assigned to the "cl" and true is returned.

  @param[out] cl        Variable to store collation.
  @param[in] cl_name    Collation name.
  @param[in] default_cl Default collation.

  @return false if collation was resolved successfully; true if there is no
  collation with given name.
*/

bool resolve_collation(const char *cl_name, const CHARSET_INFO *default_cl,
                       const CHARSET_INFO **cl) {
  *cl = get_charset_by_name(cl_name, MYF(0));

  if (*cl == nullptr) {
    *cl = default_cl;
    return true;
  }

  return false;
}

/*
  Escape string with backslashes (\)

  SYNOPSIS
    escape_string_for_mysql()
    charset_info        Charset of the strings
    to                  Buffer for escaped string
    to_length           Length of destination buffer, or 0
    from                The string to escape
    length              The length of the string to escape

  DESCRIPTION
    This escapes the contents of a string by adding backslashes before special
    characters, and turning others into specific escape sequences, such as
    turning newlines into \n and null bytes into \0.

  NOTE
    To maintain compatibility with the old C API, to_length may be 0 to mean
    "big enough"

  RETURN VALUES
    (size_t) -1 The escaped string did not fit in the to buffer
    #           The length of the escaped string
*/

size_t escape_string_for_mysql(const CHARSET_INFO *charset_info, char *to,
                               size_t to_length, const char *from,
                               size_t length) {
  const char *to_start = to;
  const char *end,
      *to_end = to_start + (to_length ? to_length - 1 : 2 * length);
  bool overflow = false;
  bool use_mb_flag = use_mb(charset_info);
  for (end = from + length; from < end; from++) {
    char escape = 0;
    int tmp_length;
    if (use_mb_flag && (tmp_length = my_ismbchar(charset_info, from, end))) {
      if (to + tmp_length > to_end) {
        overflow = true;
        break;
      }
      while (tmp_length--) *to++ = *from++;
      from--;
      continue;
    }
    /*
     If the next character appears to begin a multi-byte character, we
     escape that first byte of that apparent multi-byte character. (The
     character just looks like a multi-byte character -- if it were actually
     a multi-byte character, it would have been passed through in the test
     above.)

     Without this check, we can create a problem by converting an invalid
     multi-byte character into a valid one. For example, 0xbf27 is not
     a valid GBK character, but 0xbf5c is. (0x27 = ', 0x5c = \)
    */
    tmp_length = use_mb_flag ? my_mbcharlen_ptr(charset_info, from, end) : 0;
    if (tmp_length > 1)
      escape = *from;
    else
      switch (*from) {
        case 0: /* Must be escaped for 'mysql' */
          escape = '0';
          break;
        case '\n': /* Must be escaped for logs */
          escape = 'n';
          break;
        case '\r':
          escape = 'r';
          break;
        case '\\':
          escape = '\\';
          break;
        case '\'':
          escape = '\'';
          break;
        case '"': /* Better safe than sorry */
          escape = '"';
          break;
        case '\032': /* This gives problems on Win32 */
          escape = 'Z';
          break;
      }
    if (escape) {
      if (to + 2 > to_end) {
        overflow = true;
        break;
      }
      *to++ = '\\';
      *to++ = escape;
    } else {
      if (to + 1 > to_end) {
        overflow = true;
        break;
      }
      *to++ = *from;
    }
  }
  *to = 0;
  return overflow ? (size_t)-1 : (size_t)(to - to_start);
}

#ifdef _WIN32
static CHARSET_INFO *fs_cset_cache = NULL;

CHARSET_INFO *fs_character_set() {
  if (!fs_cset_cache) {
    char buf[10] = "cp";
    GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, buf + 2,
                  sizeof(buf) - 3);
    /*
      We cannot call get_charset_by_name here
      because fs_character_set() is executed before
      LOCK_THD_charset mutex initialization, which
      is used inside get_charset_by_name.
      As we're now interested in cp932 only,
      let's just detect it using strcmp().
    */
    fs_cset_cache =
        !strcmp(buf, "cp932") ? &my_charset_cp932_japanese_ci : &my_charset_bin;
  }
  return fs_cset_cache;
}
#endif

/*
  Escape apostrophes by doubling them up

  SYNOPSIS
    escape_quotes_for_mysql()
    charset_info        Charset of the strings
    to                  Buffer for escaped string
    to_length           Length of destination buffer, or 0
    from                The string to escape
    length              The length of the string to escape
    quote               The quote the buffer will be escaped against

  DESCRIPTION
    This escapes the contents of a string by doubling up any character
    specified by the quote parameter. This is used when the
    NO_BACKSLASH_ESCAPES SQL_MODE is in effect on the server.

  NOTE
    To be consistent with escape_string_for_mysql(), to_length may be 0 to
    mean "big enough"

  RETURN VALUES
    ~0          The escaped string did not fit in the to buffer
    >=0         The length of the escaped string
*/

size_t escape_quotes_for_mysql(CHARSET_INFO *charset_info, char *to,
                               size_t to_length, const char *from,
                               size_t length, char quote) {
  const char *to_start = to;
  const char *end,
      *to_end = to_start + (to_length ? to_length - 1 : 2 * length);
  bool overflow = false;
  bool use_mb_flag = use_mb(charset_info);
  for (end = from + length; from < end; from++) {
    int tmp_length;
    if (use_mb_flag && (tmp_length = my_ismbchar(charset_info, from, end))) {
      if (to + tmp_length > to_end) {
        overflow = true;
        break;
      }
      while (tmp_length--) *to++ = *from++;
      from--;
      continue;
    }
    /*
      We don't have the same issue here with a non-multi-byte character being
      turned into a multi-byte character by the addition of an escaping
      character, because we are only escaping the ' character with itself.
     */
    if (*from == quote) {
      if (to + 2 > to_end) {
        overflow = true;
        break;
      }
      *to++ = quote;
      *to++ = quote;
    } else {
      if (to + 1 > to_end) {
        overflow = true;
        break;
      }
      *to++ = *from;
    }
  }
  *to = 0;
  return overflow ? (ulong)~0 : (ulong)(to - to_start);
}

void charset_uninit() {
  for (CHARSET_INFO *cs : all_charsets) {
    if (cs && cs->coll->uninit) {
      cs->coll->uninit(cs);
    }
  }

  delete coll_name_num_map;
  coll_name_num_map = nullptr;
  delete cs_name_pri_num_map;
  cs_name_pri_num_map = nullptr;
  delete cs_name_bin_num_map;
  cs_name_bin_num_map = nullptr;

  new (&charsets_initialized) std::once_flag;
}
