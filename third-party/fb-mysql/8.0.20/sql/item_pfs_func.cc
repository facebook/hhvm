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

/**
  @file

  @brief
  This file defines all performance schema native functions
*/

#include "sql/item_pfs_func.h"

#include <stdio.h>
#include <cmath>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/components/services/psi_thread_bits.h"
#include "mysqld_error.h"
#include "pfs_thread_provider.h"
#include "sql/field.h"
#include "sql/item.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"

extern bool pfs_enabled;

/** ps_current_thread_id() */

bool Item_func_pfs_current_thread_id::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->safe_to_cache_query = false; /* result can vary */
  return false;
}

bool Item_func_pfs_current_thread_id::resolve_type(THD *) {
  unsigned_flag = true;
  maybe_null = true;
  return false;
}

bool Item_func_pfs_current_thread_id::fix_fields(THD *thd, Item **ref) {
  if (super::fix_fields(thd, ref)) return true;
  thd->thread_specific_used = true; /* for binlog */
  return false;
}

longlong Item_func_pfs_current_thread_id::val_int() {
  DBUG_ASSERT(fixed);
  /* Verify Performance Schema available. */
  if (!pfs_enabled) {
    my_printf_error(ER_WRONG_PERFSCHEMA_USAGE,
                    "'%s': The Performance Schema is not enabled.", MYF(0),
                    func_name());
    return error_int();
  }
#ifdef HAVE_PSI_THREAD_INTERFACE
  /* Return the thread id for this connection. */
  m_thread_id = PSI_THREAD_CALL(get_current_thread_internal_id)();
#endif
  /* Valid thread id is > 0. */
  if (m_thread_id == 0) {
    return error_int();
  }
  return static_cast<longlong>(m_thread_id);
}

/** ps_thread_id() */

bool Item_func_pfs_thread_id::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->safe_to_cache_query = false; /* result can vary */
  return false;
}

bool Item_func_pfs_thread_id::resolve_type(THD *) {
  unsigned_flag = true;
  maybe_null = true;
  return false;
}

longlong Item_func_pfs_thread_id::val_int() {
  DBUG_ASSERT(fixed);

  /* Verify Performance Schema available. */
  if (!pfs_enabled) {
    my_printf_error(ER_WRONG_PERFSCHEMA_USAGE,
                    "'%s': The Performance Schema is not enabled.", MYF(0),
                    func_name());
    return error_int();
  }

  /* Evaluate the function argument. */
  longlong processlist_id = args[0]->val_int();

  /* Verify argument type. */
  if (!is_integer_type(args[0]->data_type())) {
    return error_int();
  }

  /* If argument is null, return null. */
  null_value = args[0]->null_value;
  if (null_value) {
    return error_int();
  }

  /* Verify argument sign. */
  if (processlist_id < 0) {
    return error_int();
  }

#ifdef HAVE_PSI_THREAD_INTERFACE
  /* Get the thread id assigned to the processlist id. */
  PSI_thread *psi = PSI_THREAD_CALL(get_thread_by_id)(processlist_id);
  if (psi) {
    m_thread_id = PSI_THREAD_CALL(get_thread_internal_id)(psi);
  }
#endif

  /* Valid thread id is > 0. */
  if (m_thread_id == 0) {
    return error_int();
  }
  return static_cast<longlong>(m_thread_id);
}

/** format_bytes() */

bool Item_func_pfs_format_bytes::resolve_type(THD *) {
  maybe_null = true;
  collation.set(&my_charset_utf8_general_ci);
  /* Format is 'AAAA.BB UUU' = 11 characters or 'AAAA bytes' = 10 characters. */
  fix_char_length(11);
  return false;
}

String *Item_func_pfs_format_bytes::val_str(String *) {
  /* Evaluate argument value. */
  double bytes = args[0]->val_real();

  /* If input is null, return null. */
  null_value = args[0]->null_value;
  if (null_value) {
    return error_str();
  }

  /* Declaring 'volatile' as workaround for 32-bit optimization bug. */
  volatile double bytes_abs = std::abs(bytes);

  constexpr uint64_t kib{1024};
  constexpr uint64_t mib{1024 * kib};
  constexpr uint64_t gib{1024 * mib};
  constexpr uint64_t tib{1024 * gib};
  constexpr uint64_t pib{1024 * tib};
  constexpr uint64_t eib{1024 * pib};

  uint64_t divisor;
  int len;
  const char *unit;

  if (bytes_abs >= eib) {
    divisor = eib;
    unit = "EiB";
  } else if (bytes_abs >= pib) {
    divisor = pib;
    unit = "PiB";
  } else if (bytes_abs >= tib) {
    divisor = tib;
    unit = "TiB";
  } else if (bytes_abs >= gib) {
    divisor = gib;
    unit = "GiB";
  } else if (bytes_abs >= mib) {
    divisor = mib;
    unit = "MiB";
  } else if (bytes_abs >= kib) {
    divisor = kib;
    unit = "KiB";
  } else {
    divisor = 1;
    unit = "bytes";
  }

  if (divisor == 1) {
    len = sprintf(m_value_buffer, "%4d %s", (int)bytes, unit);
  } else {
    double value = bytes / divisor;
    if (std::abs(value) >= 100000.0) {
      len = sprintf(m_value_buffer, "%4.2e %s", value, unit);
    } else {
      len = sprintf(m_value_buffer, "%4.2f %s", value, unit);
    }
  }

  m_value.set(m_value_buffer, len, &my_charset_utf8_general_ci);
  return &m_value;
}

/** format_pico_time() */

bool Item_func_pfs_format_pico_time::resolve_type(THD *) {
  maybe_null = true;
  collation.set(&my_charset_utf8_general_ci);
  /* Format is 'AAAA.BB UUU' = 11 characters or 'AAA ps' = 6 characters. */
  fix_char_length(11);
  return false;
}

String *Item_func_pfs_format_pico_time::val_str(String *) {
  /* Evaluate the argument */
  double time_val = args[0]->val_real();

  /* If argument is null, return null. */
  null_value = args[0]->null_value;
  if (null_value) {
    return error_str();
  }

  constexpr uint64_t nano{1000};
  constexpr uint64_t micro{1000 * nano};
  constexpr uint64_t milli{1000 * micro};
  constexpr uint64_t sec{1000 * milli};
  constexpr uint64_t min{60 * sec};
  constexpr uint64_t hour{60 * min};
  constexpr uint64_t day{24 * hour};

  /* Declaring 'volatile' as workaround for 32-bit optimization bug. */
  volatile double time_abs = std::abs(time_val);

  uint64_t divisor;
  int len;
  const char *unit;

  /* SI-approved time units. */
  if (time_abs >= day) {
    divisor = day;
    unit = "d";
  } else if (time_abs >= hour) {
    divisor = hour;
    unit = "h";
  } else if (time_abs >= min) {
    divisor = min;
    unit = "min";
  } else if (time_abs >= sec) {
    divisor = sec;
    unit = "s";
  } else if (time_abs >= milli) {
    divisor = milli;
    unit = "ms";
  } else if (time_abs >= micro) {
    divisor = micro;
    unit = "us";
  } else if (time_abs >= nano) {
    divisor = nano;
    unit = "ns";
  } else {
    divisor = 1;
    unit = "ps";
  }

  if (divisor == 1) {
    len = sprintf(m_value_buffer, "%3d %s", (int)time_val, unit);
  } else {
    double value = time_val / divisor;
    if (std::abs(value) >= 100000.0) {
      len = sprintf(m_value_buffer, "%4.2e %s", value, unit);
    } else {
      len = sprintf(m_value_buffer, "%4.2f %s", value, unit);
    }
  }

  m_value.set(m_value_buffer, len, &my_charset_utf8_general_ci);
  return &m_value;
}
