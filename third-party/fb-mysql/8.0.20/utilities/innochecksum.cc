/*
   Copyright (c) 2005, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
  InnoDB offline file checksum utility.  85% of the code in this utility
  is included from the InnoDB codebase.

  The final 15% was originally written by Mark Smith of Danga
  Interactive, Inc. <junior@danga.com>

  Published with a permission.
*/

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_getopt.h"
#include "my_macros.h"
#include "prealloced_array.h"
#include "print_version.h"
#include "typelib.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

/* Only parts of these files are included from the InnoDB codebase.
The parts not included are excluded by #ifndef UNIV_INNOCHECKSUM. */

#include "storage/innobase/include/buf0checksum.h"
#include "storage/innobase/include/fil0types.h"
#include "storage/innobase/include/fsp0fsp.h"   /* fsp_flags_get_page_size() &

					   fsp_flags_get_zip_size() */
#include "storage/innobase/include/fut0lst.h"   /* FLST_NODE_SIZE */
#include "storage/innobase/include/mach0data.h" /* mach_read_from_4() */
#include "storage/innobase/include/os0file.h"
#include "storage/innobase/include/page0page.h" /* PAGE_* */
#include "storage/innobase/include/page0size.h" /* page_size_t */
#include "storage/innobase/include/page0zip.h"
#include "storage/innobase/include/trx0undo.h" /* TRX_UNDO_* */
#include "storage/innobase/include/univ.i"     /* include all of this */
#include "storage/innobase/include/ut0crc32.h" /* ut_crc32_init() */

/* Global variables */
static bool verbose;
static bool just_count;
static uintmax_t start_page;
static uintmax_t end_page;
static uintmax_t do_page;
static bool use_end_page;
static bool do_one_page;
/* replaces declaration in srv0srv.c */
ulong srv_page_size;
ulong srv_page_size_shift;
page_size_t univ_page_size(0, 0, false);
extern ulong srv_checksum_algorithm;

/* Current page number (0 based). */
uintmax_t cur_page_num;
/* Skip the checksum verification. */
static bool no_check;
/* Enabled for strict checksum verification. */
bool strict_verify = false;
/* Enabled for rewrite checksum. */
static bool do_write;
/* Mismatches count allowed (0 by default). */
static uintmax_t allow_mismatches;
static bool page_type_summary;
static bool page_type_dump;
/* Store filename for page-type-dump option. */
char *page_dump_filename = nullptr;
/* skip the checksum verification & rewrite if page is doublewrite buffer. */
static bool skip_page = false;
const char *dbug_setting = "FALSE";
char *log_filename = nullptr;
/* User defined filename for logging. */
FILE *log_file = nullptr;
/* Enabled for log write option. */
static bool is_log_enabled = false;

#ifndef _WIN32
/* advisory lock for non-window system. */
struct flock lk;
#endif /* _WIN32 */

/* Strict check algorithm name. */
static ulong strict_check;
/* Rewrite checksum algorithm name. */
static ulong write_check;

/* Innodb page type. */
struct innodb_page_type {
  int n_undo_state_active;
  int n_undo_state_cached;
  int n_undo_state_to_free;
  int n_undo_state_to_purge;
  int n_undo_state_prepared;
  int n_undo_state_other;
  int n_undo_insert;
  int n_undo_update;
  int n_undo_other;
  int n_fil_page_index;
  int n_fil_page_undo_log;
  int n_fil_page_inode;
  int n_fil_page_ibuf_free_list;
  int n_fil_page_ibuf_bitmap;
  int n_fil_page_type_sys;
  int n_fil_page_type_trx_sys;
  int n_fil_page_type_fsp_hdr;
  int n_fil_page_type_allocated;
  int n_fil_page_type_xdes;
  int n_fil_page_type_blob;
  int n_fil_page_type_zblob;
  int n_fil_page_type_other;
  int n_fil_page_type_zblob2;
  int n_fil_page_sdi_index;
  int n_fil_page_sdi_blob;
  int n_fil_page_sdi_zblob;
} page_type;

/* Possible values for "--strict-check" for strictly verify checksum
and "--write" for rewrite checksum. */
static const char *innochecksum_algorithms[] = {
    "crc32", "crc32", "innodb", "innodb", "none", "none", NullS};

/* Used to define an enumerate type of the "innochecksum algorithm". */
static TYPELIB innochecksum_algorithms_typelib = {
    array_elements(innochecksum_algorithms) - 1, "", innochecksum_algorithms,
    nullptr};

/** Error logging classes. */
namespace ib {

logger::~logger() {}

info::~info() {
  std::cerr << "[INFO] innochecksum: " << m_oss.str() << std::endl;
}

warn::~warn() {
  std::cerr << "[WARNING] innochecksum: " << m_oss.str() << std::endl;
}

error::~error() {
  std::cerr << "[ERROR] innochecksum: " << m_oss.str() << std::endl;
}

fatal::~fatal() {
  std::cerr << "[FATAL] innochecksum: " << m_oss.str() << std::endl;
  ut_error;
}
}  // namespace ib

/** Check that a page_size is correct for InnoDB. If correct, set the
associated page_size_shift which is the power of 2 for this page size.
@param[in]	page_size	page size to evaluate
@return an associated page_size_shift if valid, 0 if invalid. */
static int innodb_page_size_validate(ulong page_size) {
  ulong n;

  DBUG_TRACE;

  for (n = UNIV_PAGE_SIZE_SHIFT_MIN; n <= UNIV_PAGE_SIZE_SHIFT_MAX; n++) {
    if (page_size == (ulong)(1 << n)) {
      return n;
    }
  }

  return 0;
}

/** Get the page size of the filespace from the filespace header.
@param[in]	buf	buffer used to read the page.
@return page size */
static const page_size_t get_page_size(byte *buf) {
  const ulint flags = mach_read_from_4(buf + FIL_PAGE_DATA + FSP_SPACE_FLAGS);

  const ulint ssize = FSP_FLAGS_GET_PAGE_SSIZE(flags);

  if (ssize == 0) {
    srv_page_size = UNIV_PAGE_SIZE_ORIG;
  } else {
    srv_page_size = ((UNIV_ZIP_SIZE_MIN >> 1) << ssize);
  }

  srv_page_size_shift = innodb_page_size_validate(srv_page_size);

  ut_ad(srv_page_size_shift != 0);

  univ_page_size.copy_from(page_size_t(srv_page_size, srv_page_size, false));

  return (page_size_t(flags));
}

/** Decompress a page
@param[in,out]	buf		Page read from disk, uncompressed data will
                                also be copied to this page
@param[in, out] scratch		Page to use for temporary decompress
@param[in]	page_size	scratch physical size
@return true if decompress succeeded */
static bool page_decompress(byte *buf, byte *scratch, page_size_t page_size) {
  dberr_t err;

  /* Set the dblwr recover flag to false. */
  err = os_file_decompress_page(false, buf, scratch, page_size.physical());

  return (err == DB_SUCCESS);
}

#ifdef _WIN32
/***********************************************/ /*
  @param		[in] error	error no. from the getLastError().

  @retval error message corresponding to error no.
 */
static char *error_message(int error) {
  static char err_msg[1024] = {'\0'};
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)err_msg,
                sizeof(err_msg), NULL);

  return (err_msg);
}
#endif /* _WIN32 */

/***********************************************/ /*
  @param>>_______[in] name>_____name of file.
  @retval file pointer; file pointer is NULL when error occurred.
 */

static FILE *open_file(const char *name) {
  int fd; /* file descriptor. */
  FILE *fil_in;
#ifdef _WIN32
  HANDLE hFile;  /* handle to open file. */
  DWORD access;  /* define access control */
  int flags = 0; /* define the mode for file
                 descriptor */

  if (do_write) {
    access = GENERIC_READ | GENERIC_WRITE;
    flags = _O_RDWR | _O_BINARY;
  } else {
    access = GENERIC_READ;
    flags = _O_RDONLY | _O_BINARY;
  }
  /* CreateFile() also provide advisory lock with the usage of
  access and share mode of the file.*/
  hFile =
      CreateFile((LPCTSTR)name, access, 0L, NULL, OPEN_EXISTING, NULL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    /* print the error message. */
    fprintf(stderr, "Filename::%s %s\n", name, error_message(GetLastError()));

    return (NULL);
  }

  /* get the file descriptor. */
  fd = _open_osfhandle((intptr_t)hFile, flags);
#else  /* _WIN32 */

  int create_flag;
  /* define the advisory lock and open file mode. */
  if (do_write) {
    create_flag = O_RDWR;
    lk.l_type = F_WRLCK;
  } else {
    create_flag = O_RDONLY;
    lk.l_type = F_RDLCK;
  }

  fd = open(name, create_flag);

  lk.l_whence = SEEK_SET;
  lk.l_start = lk.l_len = 0;

  if (fcntl(fd, F_SETLK, &lk) == -1) {
    fprintf(stderr,
            "Error: Unable to lock file::"
            " %s\n",
            name);
    perror("fcntl");
    return (nullptr);
  }
#endif /* _WIN32 */

  if (do_write) {
    fil_in = fdopen(fd, "rb+");
  } else {
    fil_in = fdopen(fd, "rb");
  }

  return (fil_in);
}

/** Read the contents of file.
@param[in,out]	buf			read the file in buffer
@param[in]	partial_page_read	enable when to read the
                                       remaining buffer for first page
@param[in]	page_size		page size
@param[in,out]	fil_in			file pointer created for the
                                       tablespace
@retval number of bytes read
*/
static ulong read_file(byte *buf, bool partial_page_read,
                       const page_size_t &page_size, FILE *fil_in) {
  ulong bytes = 0;

  size_t physical_page_size = static_cast<size_t>(page_size.physical());

  DBUG_ASSERT(physical_page_size >= UNIV_ZIP_SIZE_MIN);

  if (partial_page_read) {
    buf += UNIV_ZIP_SIZE_MIN;
    physical_page_size -= UNIV_ZIP_SIZE_MIN;
    bytes = UNIV_ZIP_SIZE_MIN;
  }

  /* Nothing to read from file, just return */
  if (physical_page_size == 0) {
    return (bytes);
  }

  bytes += ulong(fread(buf, 1, physical_page_size, fil_in));

  return (bytes);
}

/** Class to check if a page is corrupted and print calculated
checksum values. */
class InnocheckReporter : public BlockReporter {
 public:
  /** Constructor
  @param[in]	check_lsn	checks lsn of the page with the
                                  current lsn (only in recovery)
  @param[in]	read_buf	buffer holding the page
  @param[in]	page_size	page size
  @param[in]	skip_checksum	skip checksum verification
  @param[in]	strict_check	true if strict checksum option enabled
  @param[in]	is_log_enabled	true if the log file is passed to
                                  innochecksum by user
  @param[in,out]	log_file	the log file to write checksum values
                                  and checksum mismatch messages */
  InnocheckReporter(bool check_lsn, const byte *read_buf,
                    const page_size_t &page_size, bool skip_checksum,
                    bool strict_check, bool is_log_enabled, FILE *log_file)
      : BlockReporter(check_lsn, read_buf, page_size, skip_checksum),
        m_strict_check(strict_check),
        m_is_log_enabled(is_log_enabled),
        m_log_file(log_file) {
    m_page_no = mach_read_from_4(read_buf + FIL_PAGE_OFFSET);
  }

  /** Print message if page is empty.
  @param[in]	empty		true if page is empty */
  virtual inline void report_empty_page(bool empty) const {
    if (empty && m_is_log_enabled) {
      fprintf(m_log_file, "Page::%" PRIuMAX " is empty and uncorrupted\n",
              m_page_no);
    }
  }

  /** Print crc32 checksum and the checksum fields in page.
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer
  @param[in]	crc32		Calculated crc32 checksum */
  virtual inline void print_strict_crc32(ulint checksum_field1,
                                         ulint checksum_field2, uint32_t crc32,
                                         srv_checksum_algorithm_t algo) const {
    if (algo != SRV_CHECKSUM_ALGORITHM_STRICT_CRC32 || !m_is_log_enabled) {
      return;
    }

    fprintf(m_log_file,
            "page::%" PRIuMAX
            ";"
            " crc32 calculated = %u;"
            " recorded checksum field1 = " ULINTPF
            " recorded checksum field2 = " ULINTPF "\n",
            m_page_no, crc32, checksum_field1, checksum_field2);
  }

  /** Print innodb checksum and the checksum fields in page.
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer */
  virtual inline void print_strict_innodb(ulint checksum_field1,
                                          ulint checksum_field2) const {
    if (!m_is_log_enabled) {
      return;
    }

    fprintf(m_log_file,
            "page::%" PRIuMAX
            ";"
            " old style: calculated = " ULINTPF "; recorded checksum = " ULINTPF
            "\n",
            m_page_no, buf_calc_page_old_checksum(m_read_buf), checksum_field2);
    fprintf(m_log_file,
            "page::%" PRIuMAX
            ";"
            " new style: calculated = " ULINTPF "; recorded checksum = " ULINTPF
            "\n",
            m_page_no, buf_calc_page_new_checksum(m_read_buf), checksum_field1);
  }

  /** Print none checksum and the checksum fields in page.
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer */
  virtual inline void print_strict_none(ulint checksum_field1,
                                        ulint checksum_field2,
                                        srv_checksum_algorithm_t algo) const {
    if (!m_is_log_enabled || algo != SRV_CHECKSUM_ALGORITHM_STRICT_NONE) {
      return;
    }

    fprintf(m_log_file,
            "page::%" PRIuMAX
            "; none checksum: calculated = %lu;"
            " recorded checksum_field1 = " ULINTPF
            " recorded checksum_field2 = " ULINTPF "\n",
            m_page_no, BUF_NO_CHECKSUM_MAGIC, checksum_field1, checksum_field2);
  }

  /** Print a message that none check failed. */
  virtual inline void print_none_fail() const {
    fprintf(m_log_file,
            "Fail; page %" PRIuMAX " invalid (fails none checksum)\n",
            m_page_no);
  }

  /** Print innodb checksum value stored in page trailer.
  @param[in]	old_checksum	checksum value according to old style
  @param[in]	new_checksum	checksum value according to new style
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer
  @param[in]	algo		current checksum algorithm */
  virtual inline void print_innodb_checksum(
      ulint old_checksum, ulint new_checksum, ulint checksum_field1,
      ulint checksum_field2, srv_checksum_algorithm_t algo) const {
    if (!m_is_log_enabled) {
      return;
    }

    switch (algo) {
      case SRV_CHECKSUM_ALGORITHM_INNODB:
        fprintf(m_log_file,
                "page::%" PRIuMAX
                ";"
                " old style: calculated = " ULINTPF "; recorded = " ULINTPF
                "\n",
                m_page_no, old_checksum, checksum_field2);
        fprintf(m_log_file,
                "page::%" PRIuMAX
                ";"
                " new style: calculated = " ULINTPF
                "; crc32 = %u; recorded = " ULINTPF "\n",
                m_page_no, new_checksum, buf_calc_page_crc32(m_read_buf),
                checksum_field1);
        break;

      case SRV_CHECKSUM_ALGORITHM_STRICT_INNODB:
        fprintf(log_file,
                "page::%" PRIuMAX
                ";"
                " old style: calculated = " ULINTPF
                "; recorded checksum = " ULINTPF "\n",
                m_page_no, old_checksum, checksum_field2);
        fprintf(log_file,
                "page::%" PRIuMAX
                ";"
                " new style: calculated = " ULINTPF
                "; recorded checksum = " ULINTPF "\n",
                m_page_no, new_checksum, checksum_field1);
        break;
      case SRV_CHECKSUM_ALGORITHM_CRC32:
      case SRV_CHECKSUM_ALGORITHM_STRICT_CRC32:
      case SRV_CHECKSUM_ALGORITHM_NONE:
      case SRV_CHECKSUM_ALGORITHM_STRICT_NONE:
        return;
    }
  }

  /** Print the message that checksum mismatch happened in
  page header. */
  virtual inline void print_innodb_fail() const {
    if (!m_is_log_enabled) {
      return;
    }

    fprintf(m_log_file,
            "Fail; page %" PRIuMAX
            " invalid (fails innodb and"
            " crc32 checksum\n",
            m_page_no);
  }

  /** Print both new-style, old-style & crc32 checksum values.
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer */
  virtual inline void print_crc32_checksum(ulint checksum_field1,
                                           ulint checksum_field2) const {
    if (m_is_log_enabled) {
      fprintf(m_log_file,
              "page::%" PRIuMAX "; old style: calculated = " ULINTPF
              "; recorded = " ULINTPF "\n",
              m_page_no, buf_calc_page_old_checksum(m_read_buf),
              checksum_field2);
      fprintf(m_log_file,
              "page::%" PRIuMAX "; new style: calculated = " ULINTPF
              "; crc32 = %u;"
              " recorded = " ULINTPF "\n",
              m_page_no, buf_calc_page_new_checksum(m_read_buf),
              buf_calc_page_crc32(m_read_buf), checksum_field1);
    }
  }

  /** Print a message that crc32 check failed. */
  virtual inline void print_crc32_fail() const {
    if (!m_is_log_enabled) {
      return;
    }

    fprintf(m_log_file,
            "Fail; page %" PRIuMAX " invalid (fails crc32 checksum)\n",
            m_page_no);
  }

  /** Print checksum values on a compressed page.
  @param[in]	calc	the calculated checksum value
  @param[in]	stored	the stored checksum in header. */
  virtual inline void print_compressed_checksum(ib_uint32_t calc,
                                                ib_uint32_t stored) const {
    if (!m_is_log_enabled) {
      return;
    }

    fprintf(m_log_file,
            "page::%" PRIuMAX
            ";"
            " %s checksum: calculated = %u;"
            " recorded = %u\n",
            m_page_no,
            buf_checksum_algorithm_name(
                static_cast<srv_checksum_algorithm_t>(srv_checksum_algorithm)),
            calc, stored);

    if (!m_strict_check) {
      return;
    }

    const uint32_t crc32 = calc_zip_checksum(SRV_CHECKSUM_ALGORITHM_CRC32);

    fprintf(m_log_file,
            "page::%" PRIuMAX
            ": crc32 checksum:"
            " calculated = %u; recorded = %u\n",
            m_page_no, crc32, stored);
    fprintf(m_log_file,
            "page::%" PRIuMAX
            ": none checksum:"
            " calculated = %lu; recorded = %u\n",
            m_page_no, BUF_NO_CHECKSUM_MAGIC, stored);
  }

 private:
  bool m_strict_check;
  bool m_is_log_enabled;
  FILE *m_log_file;
  uintmax_t m_page_no;
};

/** Check if page is corrupted or not.
@param[in]	buf		page frame
@param[in]	page_size	page size
@retval true if page is corrupted otherwise false. */
static bool is_page_corrupted(const byte *buf, const page_size_t &page_size) {
  /* use to store LSN values. */
  ulint logseq;
  ulint logseqfield;

  if (!page_size.is_compressed()) {
    /* check the stored log sequence numbers
    for uncompressed tablespace. */
    logseq = mach_read_from_4(buf + FIL_PAGE_LSN + 4);
    logseqfield = mach_read_from_4(buf + page_size.logical() -
                                   FIL_PAGE_END_LSN_OLD_CHKSUM + 4);

    if (is_log_enabled) {
      fprintf(log_file,
              "page::%" PRIuMAX "; log sequence number:first = " ULINTPF
              "; second = " ULINTPF "\n",
              cur_page_num, logseq, logseqfield);
      if (logseq != logseqfield) {
        fprintf(log_file,
                "Fail; page %" PRIuMAX
                " invalid (fails log "
                "sequence number check)\n",
                cur_page_num);
      }
    }
  }

  InnocheckReporter reporter(true, buf, page_size, false, strict_check,
                             is_log_enabled, log_file);

  return (reporter.is_corrupted());
}

/********************************************/ /*
  Check if page is doublewrite buffer or not.
  @param [in] page	buffer page

  @retval true  if page is doublewrite buffer otherwise false.
 */
static bool is_page_doublewritebuffer(const byte *page) {
  if ((cur_page_num >= FSP_EXTENT_SIZE) &&
      (cur_page_num < FSP_EXTENT_SIZE * 3)) {
    /* page is doublewrite buffer. */
    return (true);
  }

  return (false);
}

/*******************************************************/ /*
 Check if page is empty or not.
  @param		[in] page		page to checked for empty.
  @param		[in] len	size of page.

  @retval true if page is empty.
  @retval false if page is not empty.
 */
static bool is_page_empty(const byte *page, size_t len) {
  while (len--) {
    if (*page++) {
      return (false);
    }
  }
  return (true);
}

/** Rewrite the checksum for the page.
@param[in,out]	page			page buffer
@param[in]	page_size		page size in bytes on disk.
@retval		true			do rewrite
@retval		false			skip the rewrite as checksum stored
match with calculated or page is doublwrite buffer. */
static bool update_checksum(byte *page, const page_size_t &page_size) {
  size_t physical_page_size = static_cast<size_t>(page_size.physical());
  bool iscompressed = page_size.is_compressed();
  ib_uint32_t checksum = 0;
  byte stored1[4]; /* get FIL_PAGE_SPACE_OR_CHKSUM field checksum */
  byte stored2[4]; /* get FIL_PAGE_END_LSN_OLD_CHKSUM field checksum */

  ut_ad(page);
  /* If page is doublewrite buffer, skip the rewrite of checksum. */
  if (skip_page) {
    return (false);
  }

  memcpy(stored1, page + FIL_PAGE_SPACE_OR_CHKSUM, 4);
  memcpy(stored2, page + physical_page_size - FIL_PAGE_END_LSN_OLD_CHKSUM, 4);

  /* Check if page is empty, exclude the checksum field */
  if (is_page_empty(page + 4, physical_page_size - 12) &&
      is_page_empty(page + physical_page_size - 4, 4)) {
    memset(page + FIL_PAGE_SPACE_OR_CHKSUM, 0, 4);
    memset(page + physical_page_size - FIL_PAGE_END_LSN_OLD_CHKSUM, 0, 4);

    goto func_exit;
  }

  if (iscompressed) {
    /* page is compressed */
    BlockReporter reporter = BlockReporter(false, page, page_size, false);

    checksum = reporter.calc_zip_checksum(
        static_cast<srv_checksum_algorithm_t>(write_check));

    mach_write_to_4(page + FIL_PAGE_SPACE_OR_CHKSUM, checksum);
    if (is_log_enabled) {
      fprintf(log_file,
              "page::%" PRIuMAX
              "; Updated checksum ="
              " %u\n",
              cur_page_num, checksum);
    }

  } else {
    /* page is uncompressed. */

    /* Store the new formula checksum */
    switch ((srv_checksum_algorithm_t)write_check) {
      case SRV_CHECKSUM_ALGORITHM_CRC32:
      case SRV_CHECKSUM_ALGORITHM_STRICT_CRC32:
        checksum = buf_calc_page_crc32(page);
        break;

      case SRV_CHECKSUM_ALGORITHM_INNODB:
      case SRV_CHECKSUM_ALGORITHM_STRICT_INNODB:
        checksum = (ib_uint32_t)buf_calc_page_new_checksum(page);
        break;

      case SRV_CHECKSUM_ALGORITHM_NONE:
      case SRV_CHECKSUM_ALGORITHM_STRICT_NONE:
        checksum = BUF_NO_CHECKSUM_MAGIC;
        break;
        /* no default so the compiler will emit a warning if new
        enum is added and not handled here */
    }

    mach_write_to_4(page + FIL_PAGE_SPACE_OR_CHKSUM, checksum);
    if (is_log_enabled) {
      fprintf(log_file,
              "page::%" PRIuMAX
              "; Updated checksum field1"
              " = %u\n",
              cur_page_num, checksum);
    }

    if (write_check == SRV_CHECKSUM_ALGORITHM_STRICT_INNODB ||
        write_check == SRV_CHECKSUM_ALGORITHM_INNODB) {
      checksum = (ib_uint32_t)buf_calc_page_old_checksum(page);
    }

    mach_write_to_4(page + physical_page_size - FIL_PAGE_END_LSN_OLD_CHKSUM,
                    checksum);

    if (is_log_enabled) {
      fprintf(log_file,
              "page::%" PRIuMAX
              "; Updated checksum "
              "field2 = %u\n",
              cur_page_num, checksum);
    }
  }

func_exit:
  /* The following code is to check the stored checksum with the
  calculated checksum. If it matches, then return false to skip
  the rewrite of checksum, otherwise return true. */
  if (iscompressed) {
    if (!memcmp(stored1, page + FIL_PAGE_SPACE_OR_CHKSUM, 4)) {
      return (false);
    }
    return (true);
  }

  if (!memcmp(stored1, page + FIL_PAGE_SPACE_OR_CHKSUM, 4) &&
      !memcmp(stored2, page + physical_page_size - FIL_PAGE_END_LSN_OLD_CHKSUM,
              4)) {
    return (false);
  }

  return (true);
}

/** Write the content to the file
@param[in]		filename	name of the file.
@param[in,out]		file		file pointer where content
                                        have to be written
@param[in]		buf		file buffer read
@param[in,out]		pos		current file position.
@param[in]		page_size	page size
@retval			true		if successfully written
@retval			false		if a non-recoverable error occurred */
static bool write_file(const char *filename, FILE *file, byte *buf, fpos_t *pos,
                       const page_size_t &page_size) {
  bool do_update;
  ulint phys_page_size = page_size.physical();

  do_update = update_checksum(buf, page_size);

  if (file != stdin) {
    if (do_update) {
      /* Set the previous file pointer position
      saved in pos to current file position. */
      if (0 != fsetpos(file, pos)) {
        perror("fsetpos");
        return (false);
      }
    } else {
      /* Store the current file position in pos */
      if (0 != fgetpos(file, pos)) {
        perror("fgetpos");
        return (false);
      }
      return (true);
    }
  }

  if (phys_page_size !=
      fwrite(buf, 1, phys_page_size, file == stdin ? stdout : file)) {
    fprintf(stderr, "Failed to write page %" PRIuMAX " to %s: %s\n",
            cur_page_num, filename, strerror(errno));

    return (false);
  }
  if (file != stdin) {
    fflush(file);
    /* Store the current file position in pos */
    if (0 != fgetpos(file, pos)) {
      perror("fgetpos");
      return (false);
    }
  }

  return (true);
}

/*
Parse the page and collect/dump the information about page type
@param [in] page	buffer page
@param [in] file	file for diagnosis.
*/
static void parse_page(const byte *page, FILE *file) {
  unsigned long long id;
  ulint undo_page_type;
  char str[20] = {'\0'};

  /* Check whether page is doublewrite buffer. */
  if (skip_page) {
    strcpy(str, "Double_write_buffer");
  } else {
    strcpy(str, "-");
  }

  switch (mach_read_from_2(page + FIL_PAGE_TYPE)) {
    case FIL_PAGE_INDEX:
      page_type.n_fil_page_index++;
      id = mach_read_from_8(page + PAGE_HEADER + PAGE_INDEX_ID);
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tIndex page\t\t\t|"
                "\tindex id=%llu,",
                cur_page_num, id);

        fprintf(file,
                " page level=" ULINTPF ", No. of records=" ULINTPF
                ", garbage=" ULINTPF ", %s\n",
                page_header_get_field(page, PAGE_LEVEL),
                page_header_get_field(page, PAGE_N_RECS),
                page_header_get_field(page, PAGE_GARBAGE), str);
      }
      break;

    case FIL_PAGE_SDI:
      page_type.n_fil_page_sdi_index++;
      id = mach_read_from_8(page + PAGE_HEADER + PAGE_INDEX_ID);
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tSDI Index page"
                "\t\t\t|\tindex id=%llu,",
                cur_page_num, id);

        fprintf(file,
                " page level=" ULINTPF ", No. of records=" ULINTPF
                ", garbage=" ULINTPF ", %s\n",
                page_header_get_field(page, PAGE_LEVEL),
                page_header_get_field(page, PAGE_N_RECS),
                page_header_get_field(page, PAGE_GARBAGE), str);
      }
      break;

    case FIL_PAGE_UNDO_LOG:
      page_type.n_fil_page_undo_log++;
      undo_page_type =
          mach_read_from_2(page + TRX_UNDO_PAGE_HDR + TRX_UNDO_PAGE_TYPE);
      if (page_type_dump) {
        fprintf(file, "#::%8" PRIuMAX "\t\t|\t\tUndo log page\t\t\t|",
                cur_page_num);
      }
      if (undo_page_type == TRX_UNDO_INSERT) {
        page_type.n_undo_insert++;
        if (page_type_dump) {
          fprintf(file, "\t%s", "Insert Undo log page");
        }

      } else if (undo_page_type == TRX_UNDO_UPDATE) {
        page_type.n_undo_update++;
        if (page_type_dump) {
          fprintf(file, "\t%s", "Update undo log page");
        }
      }

      undo_page_type =
          mach_read_from_2(page + TRX_UNDO_SEG_HDR + TRX_UNDO_STATE);
      switch (undo_page_type) {
        case TRX_UNDO_ACTIVE:
          page_type.n_undo_state_active++;
          if (page_type_dump) {
            fprintf(file, ", %s",
                    "Undo log of "
                    "an active transaction");
          }
          break;

        case TRX_UNDO_CACHED:
          page_type.n_undo_state_cached++;
          if (page_type_dump) {
            fprintf(file, ", %s",
                    "Page is "
                    "cached for quick reuse");
          }
          break;

        case TRX_UNDO_TO_FREE:
          page_type.n_undo_state_to_free++;
          if (page_type_dump) {
            fprintf(file, ", %s",
                    "Insert undo "
                    "segment that can be freed");
          }
          break;

        case TRX_UNDO_TO_PURGE:
          page_type.n_undo_state_to_purge++;
          if (page_type_dump) {
            fprintf(file, ", %s",
                    "Will be "
                    "freed in purge when all undo"
                    "data in it is removed");
          }
          break;

        case TRX_UNDO_PREPARED:
          page_type.n_undo_state_prepared++;
          if (page_type_dump) {
            fprintf(file, ", %s",
                    "Undo log of "
                    "an prepared transaction");
          }
          break;

        default:
          page_type.n_undo_state_other++;
          break;
      }
      if (page_type_dump) {
        fprintf(file, ", %s\n", str);
      }
      break;

    case FIL_PAGE_INODE:
      page_type.n_fil_page_inode++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tInode page\t\t\t|"
                "\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_IBUF_FREE_LIST:
      page_type.n_fil_page_ibuf_free_list++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tInsert buffer free list"
                " page\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_TYPE_ALLOCATED:
      page_type.n_fil_page_type_allocated++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tFreshly allocated "
                "page\t\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_IBUF_BITMAP:
      page_type.n_fil_page_ibuf_bitmap++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tInsert Buffer "
                "Bitmap\t\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_TYPE_SYS:
      page_type.n_fil_page_type_sys++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tSystem page\t\t\t|"
                "\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_TYPE_TRX_SYS:
      page_type.n_fil_page_type_trx_sys++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tTransaction system "
                "page\t\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_TYPE_FSP_HDR:
      page_type.n_fil_page_type_fsp_hdr++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tFile Space "
                "Header\t\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_TYPE_XDES:
      page_type.n_fil_page_type_xdes++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tExtent descriptor "
                "page\t\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_TYPE_BLOB:
      page_type.n_fil_page_type_blob++;
      if (page_type_dump) {
        fprintf(file, "#::%8" PRIuMAX "\t\t|\t\tBLOB page\t\t\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_SDI_BLOB:
      page_type.n_fil_page_sdi_blob++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tSDI BLOB page"
                "\t\t\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_TYPE_ZBLOB:
      page_type.n_fil_page_type_zblob++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tCompressed BLOB "
                "page\t\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    case FIL_PAGE_TYPE_ZBLOB2:
      page_type.n_fil_page_type_zblob2++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tSubsequent Compressed "
                "BLOB page\t|\t%s\n",
                cur_page_num, str);
      }
      break;
    case FIL_PAGE_SDI_ZBLOB:
      page_type.n_fil_page_sdi_zblob++;
      if (page_type_dump) {
        fprintf(file,
                "#::%8" PRIuMAX
                "\t\t|\t\tCompressed SDI"
                " BLOB page\t|\t%s\n",
                cur_page_num, str);
      }
      break;

    default:
      page_type.n_fil_page_type_other++;
      break;
  }
}
/**
@param [in,out] file_name	name of the filename

@returns FILE pointer if successfully created else NULL when error occurred.
*/
static FILE *create_file(char *file_name) {
  FILE *file = nullptr;

#ifndef _WIN32
  file = fopen(file_name, "wb");
  if (file == nullptr) {
    fprintf(stderr, "Failed to create file: %s: %s\n", file_name,
            strerror(errno));
    return (nullptr);
  }
#else
  HANDLE hFile; /* handle to open file. */
  int fd = 0;
  hFile = CreateFile((LPCTSTR)file_name, GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, CREATE_NEW,
                     NULL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    /* print the error message. */
    fprintf(stderr, "Filename::%s %s\n", file_name,
            error_message(GetLastError()));

    return (NULL);
  }

  /* get the file descriptor. */
  fd = _open_osfhandle((intptr_t)hFile, _O_RDWR | _O_BINARY);
  file = fdopen(fd, "wb");
#endif /* _WIN32 */

  return (file);
}

/*
 Print the page type count of a tablespace.
 @param [in] fil_out	stream where the output goes.
*/
static void print_summary(FILE *fil_out) {
  fprintf(fil_out, "\n================PAGE TYPE SUMMARY==============\n");
  fprintf(fil_out, "#PAGE_COUNT\tPAGE_TYPE");
  fprintf(fil_out, "\n===============================================\n");
  fprintf(fil_out, "%8d\tIndex page\n", page_type.n_fil_page_index);
  fprintf(fil_out, "%8d\tSDI Index page\n", page_type.n_fil_page_sdi_index);
  fprintf(fil_out, "%8d\tUndo log page\n", page_type.n_fil_page_undo_log);
  fprintf(fil_out, "%8d\tInode page\n", page_type.n_fil_page_inode);
  fprintf(fil_out, "%8d\tInsert buffer free list page\n",
          page_type.n_fil_page_ibuf_free_list);
  fprintf(fil_out, "%8d\tFreshly allocated page\n",
          page_type.n_fil_page_type_allocated);
  fprintf(fil_out, "%8d\tInsert buffer bitmap\n",
          page_type.n_fil_page_ibuf_bitmap);
  fprintf(fil_out, "%8d\tSystem page\n", page_type.n_fil_page_type_sys);
  fprintf(fil_out, "%8d\tTransaction system page\n",
          page_type.n_fil_page_type_trx_sys);
  fprintf(fil_out, "%8d\tFile Space Header\n",
          page_type.n_fil_page_type_fsp_hdr);
  fprintf(fil_out, "%8d\tExtent descriptor page\n",
          page_type.n_fil_page_type_xdes);
  fprintf(fil_out, "%8d\tBLOB page\n", page_type.n_fil_page_type_blob);
  fprintf(fil_out, "%8d\tCompressed BLOB page\n",
          page_type.n_fil_page_type_zblob);
  fprintf(fil_out, "%8d\tSubsequent Compressed BLOB page\n",
          page_type.n_fil_page_type_zblob2);
  fprintf(fil_out, "%8d\tSDI BLOB page\n", page_type.n_fil_page_sdi_blob);
  fprintf(fil_out, "%8d\tCompressed SDI BLOB page\n",
          page_type.n_fil_page_sdi_zblob);
  fprintf(fil_out, "%8d\tOther type of page", page_type.n_fil_page_type_other);
  fprintf(fil_out, "\n===============================================\n");
  fprintf(fil_out, "Additional information:\n");
  fprintf(fil_out, "Undo page type: %d insert, %d update, %d other\n",
          page_type.n_undo_insert, page_type.n_undo_update,
          page_type.n_undo_other);
  fprintf(fil_out,
          "Undo page state: %d active, %d cached, %d to_free, %d"
          " to_purge, %d prepared, %d other\n",
          page_type.n_undo_state_active, page_type.n_undo_state_cached,
          page_type.n_undo_state_to_free, page_type.n_undo_state_to_purge,
          page_type.n_undo_state_prepared, page_type.n_undo_state_other);
}

/* command line argument for innochecksum tool. */
static struct my_option innochecksum_options[] = {
    {"help", '?', "Displays this help and exits.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"info", 'I', "Synonym for --help.", nullptr, nullptr, nullptr, GET_NO_ARG,
     NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"version", 'V', "Displays version information and exits.", nullptr,
     nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v', "Verbose (prints progress every 5 seconds).", &verbose,
     &verbose, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
#ifndef DBUG_OFF
    {"debug", '#', "Output debug log. See " REFMAN "dbug-package.html",
     &dbug_setting, &dbug_setting, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr,
     0, nullptr},
#endif /* !DBUG_OFF */
    {"count", 'c', "Print the count of pages in the file and exits.",
     &just_count, &just_count, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"start_page", 's', "Start on this page number (0 based).", &start_page,
     &start_page, nullptr, GET_ULL, REQUIRED_ARG, 0, 0, ULLONG_MAX, nullptr, 1,
     nullptr},
    {"end_page", 'e', "End at this page number (0 based).", &end_page,
     &end_page, nullptr, GET_ULL, REQUIRED_ARG, 0, 0, ULLONG_MAX, nullptr, 1,
     nullptr},
    {"page", 'p', "Check only this page (0 based).", &do_page, &do_page,
     nullptr, GET_ULL, REQUIRED_ARG, 0, 0, ULLONG_MAX, nullptr, 1, nullptr},
    {"strict-check", 'C', "Specify the strict checksum algorithm by the user.",
     &strict_check, &strict_check, &innochecksum_algorithms_typelib, GET_ENUM,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"no-check", 'n', "Ignore the checksum verification.", &no_check, &no_check,
     nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"allow-mismatches", 'a', "Maximum checksum mismatch allowed.",
     &allow_mismatches, &allow_mismatches, nullptr, GET_ULL, REQUIRED_ARG, 0, 0,
     ULLONG_MAX, nullptr, 1, nullptr},
    {"write", 'w', "Rewrite the checksum algorithm by the user.", &write_check,
     &write_check, &innochecksum_algorithms_typelib, GET_ENUM, REQUIRED_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"page-type-summary", 'S',
     "Display a count of each page type "
     "in a tablespace.",
     &page_type_summary, &page_type_summary, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"page-type-dump", 'D',
     "Dump the page type info for each page in a "
     "tablespace.",
     &page_dump_filename, &page_dump_filename, nullptr, GET_STR, REQUIRED_ARG,
     0, 0, 0, nullptr, 0, nullptr},
    {"log", 'l', "log output.", &log_filename, &log_filename, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},

    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

static void usage(void) {
#ifdef DBUG_OFF
  print_version();
#else
  print_version_debug();
#endif /* DBUG_OFF */
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
  printf("InnoDB offline file checksum utility.\n");
  printf(
      "Usage: %s [-c] [-s <start page>] [-e <end page>] "
      "[-p <page>] [-v]  [-a <allow mismatches>] [-n] "
      "[-C <strict-check>] [-w <write>] [-S] [-D <page type dump>] "
      "[-l <log>] <filename or [-]>\n",
      my_progname);
  printf("See " REFMAN "innochecksum.html for usage hints.\n");
  my_print_help(innochecksum_options);
  my_print_variables(innochecksum_options);
}

extern "C" bool innochecksum_get_one_option(
    int optid, const struct my_option *opt MY_ATTRIBUTE((unused)),
    char *argument MY_ATTRIBUTE((unused))) {
  switch (optid) {
#ifndef DBUG_OFF
    case '#':
      dbug_setting = argument ? argument
                              : IF_WIN("d:O,innochecksum.trace",
                                       "d:o,/tmp/innochecksum.trace");
      DBUG_PUSH(dbug_setting);
      break;
#endif /* !DBUG_OFF */
    case 'e':
      use_end_page = true;
      break;
    case 'p':
      end_page = start_page = do_page;
      use_end_page = true;
      do_one_page = true;
      break;
    case 'V':
#ifdef DBUG_OFF
      print_version();
#else
      print_version_debug();
#endif /* DBUG_OFF */
      exit(EXIT_SUCCESS);
      break;
    case 'C':
      strict_verify = true;
      switch ((srv_checksum_algorithm_t)strict_check) {
        case SRV_CHECKSUM_ALGORITHM_STRICT_CRC32:
        case SRV_CHECKSUM_ALGORITHM_CRC32:
          srv_checksum_algorithm = SRV_CHECKSUM_ALGORITHM_STRICT_CRC32;
          break;

        case SRV_CHECKSUM_ALGORITHM_STRICT_INNODB:
        case SRV_CHECKSUM_ALGORITHM_INNODB:
          srv_checksum_algorithm = SRV_CHECKSUM_ALGORITHM_STRICT_INNODB;
          break;

        case SRV_CHECKSUM_ALGORITHM_STRICT_NONE:
        case SRV_CHECKSUM_ALGORITHM_NONE:
          srv_checksum_algorithm = SRV_CHECKSUM_ALGORITHM_STRICT_NONE;
          break;
        default:
          return (true);
      }
      break;
    case 'n':
      no_check = true;
      break;
    case 'a':
    case 'S':
      break;
    case 'w':
      do_write = true;
      break;
    case 'D':
      page_type_dump = true;
      break;
    case 'l':
      is_log_enabled = true;
      break;
    case 'I':
    case '?':
      usage();
      exit(EXIT_SUCCESS);
      break;
  }

  return (false);
}

static bool get_options(int *argc, char ***argv) {
  if (handle_options(argc, argv, innochecksum_options,
                     innochecksum_get_one_option))
    exit(true);

  /* The next arg must be the filename */
  if (!*argc) {
    usage();
    return (true);
  }

  return (false);
}

int main(int argc, char **argv) {
  /* our input file. */
  FILE *fil_in = nullptr;
  /* our input filename. */
  char *filename;
  /* Buffer to store pages read. */
  Prealloced_array<byte, 1> buf(PSI_NOT_INSTRUMENTED);
  /* bytes read count */
  ulong bytes;
  /* Buffer to decompress page.*/
  byte *tbuf = nullptr;
  /* current time */
  time_t now;
  /* last time */
  time_t lastt;
  /* stat, to get file size. */
#ifdef _WIN32
  struct _stat64 st;
#else
  struct stat st;
#endif /* _WIN32 */

  /* size of file (has to be 64 bits) */
  unsigned long long int size = 0;
  /* number of pages in file */
  ulint pages;

  off_t offset = 0;
  /* count the no. of page corrupted. */
  ulint mismatch_count = 0;
  /* Variable to ack the page is corrupted or not. */
  bool is_corrupted = false;

  bool partial_page_read = false;
  /* Enabled when read from stdin is done. */
  bool read_from_stdin = false;
  FILE *fil_page_type = nullptr;
  fpos_t pos;

  /* Use to check the space id of given file. If space_id is zero,
  then check whether page is doublewrite buffer.*/
  ulint space_id = 0UL;
  /* enable when space_id of given file is zero. */
  bool is_system_tablespace = false;

  ut_crc32_init();
  MY_INIT(argv[0]);

  DBUG_TRACE;
  DBUG_PROCESS(argv[0]);

  if (get_options(&argc, &argv)) {
    return 1;
  }

  if (strict_verify && no_check) {
    fprintf(stderr,
            "Error: --strict-check option cannot be used "
            "together with --no-check option.\n");
    return 1;
  }

  if (no_check && !do_write) {
    fprintf(stderr,
            "Error: --no-check must be associated with "
            "--write option.\n");
    return 1;
  }

  if (page_type_dump) {
    fil_page_type = create_file(page_dump_filename);
    if (!fil_page_type) {
      return 1;
    }
  }

  if (is_log_enabled) {
    log_file = create_file(log_filename);
    if (!log_file) {
      return 1;
    }
    fprintf(log_file, "InnoDB File Checksum Utility.\n");
  }

  if (verbose) {
    my_print_variables_ex(innochecksum_options, stderr);
  }

  buf.reserve(UNIV_PAGE_SIZE_MAX * 2);
  tbuf = buf.begin() + UNIV_PAGE_SIZE_MAX;

  /* The file name is not optional. */
  for (int i = 0; i < argc; ++i) {
    /* Reset parameters for each file. */
    filename = argv[i];
    memset(&page_type, 0, sizeof(innodb_page_type));
    is_corrupted = false;
    partial_page_read = false;
    skip_page = false;

    if (is_log_enabled) {
      fprintf(log_file, "Filename = %s\n", filename);
    }

    if (*filename == '-') {
      /* read from stdin. */
      fil_in = stdin;
      read_from_stdin = true;
    }

    /* stat the file to get size and page count. */
    if (!read_from_stdin &&
#ifdef _WIN32
        _stat64(filename, &st)) {
#else
        stat(filename, &st)) {
#endif /* _WIN32 */
      fprintf(stderr, "Error: %s cannot be found\n", filename);

      return 1;
    }

    if (!read_from_stdin) {
      size = st.st_size;
      fil_in = open_file(filename);
      /*If fil_in is NULL, terminate as some error encountered */
      if (fil_in == nullptr) {
        return 1;
      }
      /* Save the current file pointer in pos variable.*/
      if (0 != fgetpos(fil_in, &pos)) {
        perror("fgetpos");
        return 1;
      }
    }

    /* Testing for lock mechanism. The innochecksum
    acquire lock on given file. So other tools accessing the same
    file for processsing must fail. */
#ifdef _WIN32
    DBUG_EXECUTE_IF(
        "innochecksum_cause_mysqld_crash", ut_ad(page_dump_filename);
        while ((_access(page_dump_filename, 0)) == 0) { sleep(1); } return 0;);
#else
    DBUG_EXECUTE_IF(
        "innochecksum_cause_mysqld_crash", ut_ad(page_dump_filename);
        struct stat status_buf; while (stat(page_dump_filename, &status_buf) ==
                                       0) { sleep(1); } return 0;);
#endif /* _WIN32 */

    /* Read the minimum page size. */
    bytes = ulong(fread(buf.begin(), 1, UNIV_ZIP_SIZE_MIN, fil_in));
    partial_page_read = true;

    if (bytes != UNIV_ZIP_SIZE_MIN) {
      fprintf(stderr,
              "Error: Was not able to read the "
              "minimum page size ");
      fprintf(stderr, "of %d bytes.  Bytes read was %lu\n", UNIV_ZIP_SIZE_MIN,
              bytes);

      return 1;
    }

    /* enable variable is_system_tablespace when space_id of given
    file is zero. Use to skip the checksum verification and rewrite
    for doublewrite pages. */
    is_system_tablespace =
        (!memcmp(&space_id, buf.begin() + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID, 4))
            ? true
            : false;

    const page_size_t &page_size = get_page_size(buf.begin());

    pages = (ulint)(size / page_size.physical());

    if (just_count) {
      if (read_from_stdin) {
        fprintf(stderr, "Number of pages:" ULINTPF "\n", pages);
      } else {
        printf("Number of pages:" ULINTPF "\n", pages);
      }
      continue;
    } else if (verbose && !read_from_stdin) {
      if (is_log_enabled) {
        fprintf(log_file,
                "file %s = %llu bytes "
                "(" ULINTPF " pages)\n",
                filename, size, pages);
        if (do_one_page) {
          fprintf(log_file,
                  "Innochecksum: "
                  "checking page %" PRIuMAX "\n",
                  do_page);
        }
      }
    } else {
      if (is_log_enabled) {
        fprintf(log_file,
                "Innochecksum: checking "
                "pages in range %" PRIuMAX " to %" PRIuMAX "\n",
                start_page, use_end_page ? end_page : (pages - 1));
      }
    }

    /* seek to the necessary position */
    if (start_page) {
      if (!read_from_stdin) {
        /* If read is not from stdin, we can use
        fseeko() to position the file pointer to
        the desired page. */
        partial_page_read = false;

        offset = (off_t)start_page * (off_t)page_size.physical();
#ifdef _WIN32
        if (_fseeki64(fil_in, offset, SEEK_SET)) {
#else
        if (fseeko(fil_in, offset, SEEK_SET)) {
#endif /* _WIN32 */
          perror(
              "Error: Unable to seek to "
              "necessary offset");

          return 1;
        }
        /* Save the current file pointer in
        pos variable. */
        if (0 != fgetpos(fil_in, &pos)) {
          perror("fgetpos");

          return 1;
        }
      } else {
        ulong count = 0;

        while (!feof(fil_in)) {
          if (start_page == count) {
            break;
          }
          /* We read a part of page to find the
          minimum page size. We cannot reset
          the file pointer to the beginning of
          the page if we are reading from stdin
          (fseeko() on stdin doesn't work). So
          read only the remaining part of page,
          if partial_page_read is enable. */
          bytes = read_file(buf.begin(), partial_page_read, page_size, fil_in);

          partial_page_read = false;
          count++;

          if (!bytes || feof(fil_in)) {
            fprintf(stderr,
                    "Error: Unable "
                    "to seek to necessary "
                    "offset");

            return 1;
          }
        }
      }
    }

    if (page_type_dump) {
      fprintf(fil_page_type, "\n\nFilename::%s\n", filename);
      fprintf(fil_page_type,
              "========================================"
              "======================================\n");
      fprintf(fil_page_type,
              "\tPAGE_NO\t\t|\t\tPAGE_TYPE\t\t"
              "\t|\tEXTRA INFO\n");
      fprintf(fil_page_type,
              "========================================"
              "======================================\n");
    }

    /* main checksumming loop */
    cur_page_num = start_page;
    lastt = 0;
    while (!feof(fil_in)) {
      bytes = read_file(buf.begin(), partial_page_read, page_size, fil_in);
      partial_page_read = false;

      if (!bytes && feof(fil_in)) {
        break;
      }

      if (ferror(fil_in)) {
        fprintf(stderr, "Error reading %zu bytes", page_size.physical());
        perror(" ");

        return 1;
      }

      if (bytes != page_size.physical()) {
        fprintf(stderr,
                "Error: bytes read (%lu) "
                "doesn't match page size (%zu)\n",
                bytes, page_size.physical());
        return 1;
      }

      if (is_system_tablespace) {
        /* enable when page is double write buffer.*/
        skip_page = is_page_doublewritebuffer(buf.begin());
      } else {
        skip_page = false;

        if (!page_decompress(buf.begin(), tbuf, page_size)) {
          fprintf(stderr, "Page decompress failed");

          return 1;
        }
      }

      /* If no-check is enabled, skip the
      checksum verification.*/
      if (!no_check) {
        /* Checksum verification */
        if (!skip_page) {
          is_corrupted = is_page_corrupted(buf.begin(), page_size);

          if (is_corrupted) {
            fprintf(stderr,
                    "Fail: page "
                    "%" PRIuMAX " invalid\n",
                    cur_page_num);

            mismatch_count++;

            if (mismatch_count > allow_mismatches) {
              fprintf(stderr,
                      "Exceeded the "
                      "maximum allowed "
                      "checksum mismatch "
                      "count::%" PRIuMAX "\n",
                      allow_mismatches);

              return 1;
            }
          }
        }
      }

      /* Rewrite checksum */
      if (do_write &&
          !write_file(filename, fil_in, buf.begin(), &pos, page_size)) {
        return 1;
      }

      /* end if this was the last page we were supposed to check */
      if (use_end_page && (cur_page_num >= end_page)) {
        break;
      }

      if (page_type_summary || page_type_dump) {
        parse_page(buf.begin(), fil_page_type);
      }

      /* do counter increase and progress printing */
      cur_page_num++;
      if (verbose && !read_from_stdin) {
        if ((cur_page_num % 64) == 0) {
          now = time(nullptr);
          if (!lastt) {
            lastt = now;
          }
          if (now - lastt >= 1 && is_log_enabled) {
            fprintf(log_file,
                    "page %" PRIuMAX
                    " "
                    "okay: %.3f%% done\n",
                    (cur_page_num - 1), (float)cur_page_num / pages * 100);
            lastt = now;
          }
        }
      }
    }

    if (!read_from_stdin) {
      /* flcose() will flush the data and release the lock if
      any acquired. */
      fclose(fil_in);
    }

    /* Enabled for page type summary. */
    if (page_type_summary) {
      if (!read_from_stdin) {
        fprintf(stdout, "\nFile::%s", filename);
        print_summary(stdout);
      } else {
        print_summary(stderr);
      }
    }
  }

  if (is_log_enabled) {
    fclose(log_file);
  }

  return 0;
}

/** Report a failed assertion
@param[in]	expr	the failed assertion (optional)
@param[in]	file	source file containting the assertion
@param[in]	line	line number of the assertion */
void ut_dbg_assertion_failed(const char *expr, const char *file, ulint line) {
  fprintf(stderr,
          "Innochecksum: Assertion failure in"
          " file %s line " ULINTPF "\n",
          file, line);

  if (expr) {
    fprintf(stderr, "Innochecksum: Failing assertion: %s\n", expr);
  }

  fflush(stderr);
  fflush(stdout);
  abort();
}
