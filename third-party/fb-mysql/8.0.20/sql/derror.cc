/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/derror.h"

#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/log.h"
#include "sql/mysqld.h"  // lc_messages_dir
#include "sql/psi_memory_key.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_locale.h"
#include "sql/system_variables.h"
#include "storage/perfschema/pfs_error.h"

CHARSET_INFO *error_message_charset_info;

static const char *ERRMSG_FILE = "errmsg.sys";

/**
  Find the error message record for a given MySQL error code in the
  array of registered messages.
  The result is an index for said array; this value should not be
  considered stable between subsequent invocations of the server.

  @param mysql_errno  the error code to look for

  @retval -1          no message registered for this error code
  @retval >=0         index
 */
int mysql_errno_to_builtin(uint mysql_errno) {
  int offset = 0;  // Position where the current section starts in the array.
  int i;
  int temp_errno = (int)mysql_errno;

  for (i = 0; i < NUM_SECTIONS; i++) {
    if (temp_errno >= errmsg_section_start[i] &&
        temp_errno < (errmsg_section_start[i] + errmsg_section_size[i]))
      return mysql_errno - errmsg_section_start[i] + offset;
    offset += errmsg_section_size[i];
  }
  return -1; /* General error */
}

/*
  Error messages are stored sequentially in an array.
  But logically error messages are organized in sections where
  each section contains errors that are consecutively numbered.
  This function maps from a "logical" mysql_errno to an array
  index and returns the string.
*/
const char *MY_LOCALE_ERRMSGS::lookup(int mysql_errno) {
  int offset = 0;  // Position where the current section starts in the array.
  for (int i = 0; i < NUM_SECTIONS; i++) {
    if (mysql_errno >= errmsg_section_start[i] &&
        mysql_errno < (errmsg_section_start[i] + errmsg_section_size[i]))
      return errmsgs[mysql_errno - errmsg_section_start[i] + offset];
    offset += errmsg_section_size[i];
  }
  return "Invalid error code";
}

#ifndef CHECK_ERRMSG_FORMAT
const char *ER_DEFAULT(int mysql_errno) {
  return my_default_lc_messages->errmsgs->lookup(mysql_errno);
}

const char *ER_THD(const THD *thd, int mysql_errno) {
  return thd->variables.lc_messages->errmsgs->lookup(mysql_errno);
}
#endif  // CHECK_ERRMSG_FORMAT
const char *ER_DEFAULT_NONCONST(int mysql_errno) {
  return my_default_lc_messages->errmsgs->lookup(mysql_errno);
}

const char *ER_THD_NONCONST(const THD *thd, int mysql_errno) {
  return thd->variables.lc_messages->errmsgs->lookup(mysql_errno);
}

/**
  Get the error-message corresponding to the given MySQL error-code,
  or nullptr if no message is available for that code (this may indicate
  a bug in the caller).

  If error messages have been loaded, return the appropriate message
  in the configured default language; otherwise, retrieve the message
  from the compiled-in set (in English).

  @param   mysql_errno    MySQL error-code
  @retval  an error-message if available, or nullptr
*/
static const char *error_message_fetch(int mysql_errno) {
  if ((my_default_lc_messages != nullptr) &&
      (my_default_lc_messages->errmsgs->is_loaded()))
    return ER_DEFAULT_NONCONST(mysql_errno);

  {
    server_error *sqlstate_map = &error_names_array[1];
    int i = mysql_errno_to_builtin(mysql_errno);

    if (i >= 0) return sqlstate_map[i].text;
  }

  return nullptr;
}

/**
  Get the error-message corresponding to the given MySQL error-code,
  or nullptr if no message is available for that code (this may indicate
  a bug in the caller).

  Use this variant for messages intended for the server's error-log.

  If error messages have been loaded, return the appropriate message
  in the configured default language; otherwise, retrieve the message
  from the compiled-in set (in English).

  @param   mysql_errno    MySQL error-code
  @retval  an error-message if available, or nullptr
*/
const char *error_message_for_error_log(int mysql_errno) {
  return error_message_fetch(mysql_errno);
}

/**
  Get the error-message corresponding to the given MySQL error-code,
  or nullptr if no message is available for that code (this may indicate
  a bug in the caller).

  Use this variant for messages intended for sending to a client.

  If the session language is known, the message will be returned
  in that language if available; otherwise, we will fall back on
  the configured default language if loaded, or finally on the
  built-in default, English.

  @param   mysql_errno    MySQL error-code
  @retval  an error-message if available, or nullptr
*/
const char *error_message_for_client(int mysql_errno) {
  if (current_thd) return ER_THD_NONCONST(current_thd, mysql_errno);

  return error_message_fetch(mysql_errno);
}

bool init_errmessage() {
  DBUG_TRACE;

  /* Read messages from file. */
  (void)my_default_lc_messages->errmsgs->read_texts();

  if (!my_default_lc_messages->errmsgs->is_loaded())
    return true; /* Fatal error, not able to allocate memory. */

  /* Register messages for use with my_error(). */
  for (int i = 0; i < NUM_SECTIONS; i++) {
    if (my_error_register(
            error_message_for_client, errmsg_section_start[i],
            errmsg_section_start[i] + errmsg_section_size[i] - 1)) {
      my_default_lc_messages->errmsgs->destroy();
      return true;
    }
  }

  return false;
}

void deinit_errmessage() {
  for (int i = 0; i < NUM_SECTIONS; i++) {
    my_error_unregister(errmsg_section_start[i],
                        errmsg_section_start[i] + errmsg_section_size[i] - 1);
  }
}

/**
  Read text from packed textfile in language-directory.

  @retval false          On success
  @retval true           On failure

  @note If we can't read messagefile then it's panic- we can't continue.
*/

bool MY_LOCALE_ERRMSGS::read_texts() {
  uint no_of_errmsgs;
  size_t length;
  File file;
  char name[FN_REFLEN];
  char lang_path[FN_REFLEN];
  uchar *start_of_errmsgs = nullptr;
  uchar head[32];
  uint error_messages = 0;

  DBUG_TRACE;

  for (int i = 0; i < NUM_SECTIONS; i++)
    error_messages += errmsg_section_size[i];

  convert_dirname(lang_path, language, NullS);
  (void)my_load_path(lang_path, lang_path, lc_messages_dir);
  if ((file = mysql_file_open(key_file_ERRMSG,
                              fn_format(name, ERRMSG_FILE, lang_path, "", 4),
                              O_RDONLY, MYF(0))) < 0) {
    /*
      Trying pre-5.5 sematics of the --language parameter.
      It included the language-specific part, e.g.:

      --language=/path/to/english/
    */
    if ((file = mysql_file_open(
             key_file_ERRMSG,
             fn_format(name, ERRMSG_FILE, lc_messages_dir, "", 4), O_RDONLY,
             MYF(0))) < 0) {
      LogErr(ERROR_LEVEL, ER_ERRMSG_CANT_FIND_FILE, name);
      goto open_err;
    }

    LogErr(WARNING_LEVEL, ER_ERRMSG_LOADING_55_STYLE, lc_messages_dir);
  }

  // Read the header from the file
  if (mysql_file_read(file, (uchar *)head, 32, MYF(MY_NABP))) goto read_err;
  if (head[0] != (uchar)254 || head[1] != (uchar)254 || head[2] != 3 ||
      head[3] != 1 || head[4] != 1)
    goto read_err;

  error_message_charset_info = system_charset_info;
  length = uint4korr(head + 6);
  no_of_errmsgs = uint4korr(head + 10);

  if (no_of_errmsgs < error_messages) {
    LogErr(ERROR_LEVEL, ER_ERRMSG_MISSING_IN_FILE, name, no_of_errmsgs,
           error_messages);
    (void)mysql_file_close(file, MYF(MY_WME));
    goto open_err;
  }

  // Free old language and allocate for the new one
  my_free(errmsgs);
  if (!(errmsgs = (const char **)my_malloc(
            key_memory_errmsgs, length + no_of_errmsgs * sizeof(char *),
            MYF(0)))) {
    LogErr(ERROR_LEVEL, ER_ERRMSG_OOM, name);
    (void)mysql_file_close(file, MYF(MY_WME));
    return true;
  }

  // Get pointer to Section2.
  start_of_errmsgs = (uchar *)(errmsgs + no_of_errmsgs);

  /*
    Temporarily read message offsets into Section2.
    We cannot read these 4 byte offsets directly into Section1,
    as pointer size vary between processor architecture.
  */
  if (mysql_file_read(file, start_of_errmsgs, (size_t)no_of_errmsgs * 4,
                      MYF(MY_NABP)))
    goto read_err_init;

  // Copy the message offsets to Section1.
  {
    const uchar *pos = start_of_errmsgs;
    for (uint i = 0; i < no_of_errmsgs; i++) {
      errmsgs[i] = pointer_cast<char *>(start_of_errmsgs) + uint4korr(pos);
      pos += 4;
    }
  }

  // Copy all the error text messages into Section2.
  if (mysql_file_read(file, start_of_errmsgs, length, MYF(MY_NABP)))
    goto read_err_init;

  (void)mysql_file_close(file, MYF(0));

  return false;

read_err_init:
  /*
    At this point, we've already thrown away any old, valid setup
    we may have had, and we have a half set up message-set.
    Release the mess and fall through to init from built-ins below!
  */
  my_free(errmsgs);
  errmsgs = nullptr;
read_err:
  LogErr(ERROR_LEVEL, ER_ERRMSG_CANT_READ, name);
  (void)mysql_file_close(file, MYF(MY_WME));
open_err:
  /*
    We may have failed now, but we may still have succeeded earlier,
    so check whether we've got errmsgs from the previous time!
  */
  if (!errmsgs) {
    /*
      If we can't read the messages from disk, allocate space just for
      the pointers, and set up pointers to reference our built-in defaults
      for the messages. Since (messages + pointers) is allocated and freed
      as one contiguous memory block, this will still be released correctly
      at shutdown.
    */
    if ((errmsgs = (const char **)my_malloc(
             key_memory_errmsgs, error_messages * sizeof(char *), MYF(0)))) {
      server_error *sqlstate_map = &error_names_array[1];

      for (uint i = 0; i < error_messages; ++i)
        errmsgs[i] = sqlstate_map[i].text;
    }
  }

  return true;
} /* read_texts */

void MY_LOCALE_ERRMSGS::destroy() {
  my_free(errmsgs);
  errmsgs = nullptr;
}
