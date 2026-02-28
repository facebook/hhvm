/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_ALTER_INSTANCE_INCLUDED
#define SQL_ALTER_INSTANCE_INCLUDED

class THD;
/*
  Base class for execution control for ALTER INSTANCE ... statement
*/
class Alter_instance {
 protected:
  THD *m_thd;

 public:
  explicit Alter_instance(THD *thd) : m_thd(thd) {}
  virtual bool execute() = 0;
  bool log_to_binlog();
  virtual ~Alter_instance() {}
};

class Rotate_innodb_master_key : public Alter_instance {
 public:
  explicit Rotate_innodb_master_key(THD *thd) : Alter_instance(thd) {}

  bool execute();
  ~Rotate_innodb_master_key() {}
};

class Rotate_binlog_master_key : public Alter_instance {
 public:
  explicit Rotate_binlog_master_key(THD *thd) : Alter_instance(thd) {}

  /**
    Executes master key rotation by calling Rpl_encryption api.

    @retval False on success
    @retval True on error
  */
  bool execute();
  virtual ~Rotate_binlog_master_key() = default;
};

#endif /* SQL_ALTER_INSTANCE_INCLUDED */
