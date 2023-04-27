/* Copyright (c) 2008, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_SIGNAL_H
#define SQL_SIGNAL_H

#include <string.h>

#include "my_sqlcommand.h"
#include "sql/sql_cmd.h"    // Sql_cmd
#include "sql/sql_error.h"  // Sql_condition

class Item;
class THD;
class sp_condition_value;

/**
   This enumeration list all the condition item names of a condition in the
   SQL condition area.
*/
enum enum_condition_item_name {
  /*
    Conditions that can be set by the user (SIGNAL/RESIGNAL),
    and by the server implementation.
  */

  CIN_CLASS_ORIGIN = 0,
  CIN_FIRST_PROPERTY = CIN_CLASS_ORIGIN,
  CIN_SUBCLASS_ORIGIN = 1,
  CIN_CONSTRAINT_CATALOG = 2,
  CIN_CONSTRAINT_SCHEMA = 3,
  CIN_CONSTRAINT_NAME = 4,
  CIN_CATALOG_NAME = 5,
  CIN_SCHEMA_NAME = 6,
  CIN_TABLE_NAME = 7,
  CIN_COLUMN_NAME = 8,
  CIN_CURSOR_NAME = 9,
  CIN_MESSAGE_TEXT = 10,
  CIN_MYSQL_ERRNO = 11,
  CIN_LAST_PROPERTY = CIN_MYSQL_ERRNO
};

/**
  Set_signal_information is a container used in the parsed tree to represent
  the collection of assignments to condition items in the SIGNAL and RESIGNAL
  statements.
*/
class Set_signal_information {
 public:
  Set_signal_information() { memset(m_item, 0, sizeof(m_item)); }

  ~Set_signal_information() {}

  bool set_item(enum_condition_item_name name, Item *item);

  /**
    For each condition item assignment, m_item[] contains the parsed tree
    that represents the expression assigned, if any.
    m_item[] is an array indexed by enum_condition_item_name.
  */
  Item *m_item[CIN_LAST_PROPERTY + 1];
};

/**
  Sql_cmd_common_signal represents the common properties of the
  SIGNAL and RESIGNAL statements.
*/
class Sql_cmd_common_signal : public Sql_cmd {
 protected:
  /**
    Constructor.
    @param cond the condition signaled if any, or NULL.
    @param set collection of signal condition item assignments.
  */
  Sql_cmd_common_signal(const sp_condition_value *cond,
                        Set_signal_information *set)
      : Sql_cmd(), m_cond(cond), m_set_signal_information(set) {}

  virtual ~Sql_cmd_common_signal() {}

  /**
    Assign the condition items 'MYSQL_ERRNO', 'level' and 'MESSAGE_TEXT'
    default values of a condition.
    @param thd the current thread.
    @param cond the condition to update.
    @param set_level_code true if 'level' and 'MYSQL_ERRNO' needs to be
    overwritten
    @param level the level to assign
    @param sqlcode the sql code to assign
  */
  static void assign_defaults(THD *thd, Sql_condition *cond,
                              bool set_level_code,
                              Sql_condition::enum_severity_level level,
                              int sqlcode);

  /**
    Evaluate the condition items 'SQLSTATE', 'MYSQL_ERRNO', 'level' and
    'MESSAGE_TEXT' default values for this statement.
    @param thd the current thread.
    @param cond the condition to update.
  */
  void eval_defaults(THD *thd, Sql_condition *cond);

  /**
    Evaluate each signal condition items for this statement.
    @param thd the current thread.
    @param cond the condition to update.
    @return 0 on success.
  */
  int eval_signal_informations(THD *thd, Sql_condition *cond);

  /**
    The condition to signal or resignal.
    This member is optional and can be NULL (RESIGNAL).
  */
  const sp_condition_value *m_cond;

  /**
    Collection of 'SET item = value' assignments in the
    SIGNAL/RESIGNAL statement.
  */
  Set_signal_information *m_set_signal_information;
};

/**
  Sql_cmd_signal represents a SIGNAL statement.
*/
class Sql_cmd_signal : public Sql_cmd_common_signal {
 public:
  /**
    Constructor, used to represent a SIGNAL statement.
    @param cond the SQL condition to signal (required).
    @param set the collection of signal informations to signal.
  */
  Sql_cmd_signal(const sp_condition_value *cond, Set_signal_information *set)
      : Sql_cmd_common_signal(cond, set) {}

  virtual ~Sql_cmd_signal() {}

  virtual enum_sql_command sql_command_code() const { return SQLCOM_SIGNAL; }

  virtual bool execute(THD *thd);
};

/**
  Sql_cmd_resignal represents a RESIGNAL statement.
*/
class Sql_cmd_resignal : public Sql_cmd_common_signal {
 public:
  /**
    Constructor, used to represent a RESIGNAL statement.
    @param cond the SQL condition to resignal (optional, may be NULL).
    @param set the collection of signal informations to resignal.
  */
  Sql_cmd_resignal(const sp_condition_value *cond, Set_signal_information *set)
      : Sql_cmd_common_signal(cond, set) {}

  virtual ~Sql_cmd_resignal() {}

  virtual enum_sql_command sql_command_code() const { return SQLCOM_RESIGNAL; }

  virtual bool execute(THD *thd);
};

#endif
