/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_TABLESPACE_INCLUDED
#define SQL_TABLESPACE_INCLUDED

#include <sys/types.h>

#include "lex_string.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "sql/handler.h"  // ts_command_type
#include "sql/sql_cmd.h"  // Sql_cmd

class THD;

/**
  Structure used by parser to store options for tablespace statements
  and pass them on to Excution classes.
 */
struct Tablespace_options {
  ulonglong extent_size = 1024 * 1024;           // Default 1 MByte
  ulonglong undo_buffer_size = 8 * 1024 * 1024;  // Default 8 MByte
  ulonglong redo_buffer_size = 8 * 1024 * 1024;  // Default 8 MByte
  ulonglong initial_size = 128 * 1024 * 1024;    // Default 128 MByte
  ulonglong autoextend_size = 0;                 // No autoextension as default
  ulonglong max_size = 0;         // Max size == initial size => no extension
  ulonglong file_block_size = 0;  // 0=default or must be a valid Page Size
  uint nodegroup_id = UNDEF_NODEGROUP;
  bool wait_until_completed = true;
  LEX_STRING ts_comment = {nullptr, 0};  // FIXME: Rename to comment?
  LEX_CSTRING engine_name = {nullptr, 0};
  LEX_STRING encryption = {nullptr, 0};
};

/**
  Check if tablespace name has valid length.

  @param tablespace_name        Name of the tablespace

  @note Tablespace names are not reflected in the file system, so
        character case conversion or consideration is not relevant.

  @note Checking for path characters or ending space is not done.
        The checks are for identifier length, both in terms of
        number of characters and number of bytes.

  @retval  false   No error encountered while checking length.
  @retval  true    Error encountered and reported.
*/

bool validate_tablespace_name_length(const char *tablespace_name);

/**
  Check if a tablespace name is valid.

  SE specific validation is done by the SE by invoking a handlerton method.

  @param ts_cmd                 Whether this is tablespace DDL or not.
  @param tablespace_name        Name of the tablespace
  @param engine                 Handlerton for the tablespace.

  @retval  false   No error encountered while checking the name.
  @retval  true    Error encountered and reported.
*/

bool validate_tablespace_name(ts_command_type ts_cmd,
                              const char *tablespace_name,
                              const handlerton *engine);

/**
  Base class for tablespace execution classes including
  CREATE/ALTER/DROP TABLESPACE and LOGFILE GROUP commands.
 */
class Sql_cmd_tablespace : public Sql_cmd /* purecov: inspected */
{
 protected:
  const LEX_STRING m_tablespace_name;
  const Tablespace_options *m_options;

  /**
    Creates shared base object.

    @param  name      name of tablespace
    @param  options   additional options to statement
   */
  Sql_cmd_tablespace(const LEX_STRING &name, const Tablespace_options *options);

 public:
  /**
    Provide access to the command code enum value.
    @return command code enum value
   */
  enum_sql_command sql_command_code() const override final;
  /**
    Return the Tablespace_options for this object.
   */
  const Tablespace_options get_options() const { return *m_options; }
};

/**
  Execution class for CREATE TABLESPACE ... ADD DATAFILE ...
 */
class Sql_cmd_create_tablespace final
    : public Sql_cmd_tablespace /* purecov: inspected */
{
  const LEX_STRING m_datafile_name;
  const LEX_STRING m_logfile_group_name;
  bool m_auto_generate_datafile_name;

 public:
  /**
    Creates execution class instance for create tablespace statement.

    @param  tsname    name of tablespace
    @param  dfname    name of data file
    @param  lfgname   name of logfile group (may be {nullptr, 0})
    @param  options   additional options to statement
  */
  Sql_cmd_create_tablespace(const LEX_STRING &tsname, const LEX_STRING &dfname,
                            const LEX_STRING &lfgname,
                            const Tablespace_options *options);

  bool execute(THD *) override;
};

/**
  Execution class for DROP TABLESPACE ...
 */
class Sql_cmd_drop_tablespace final
    : public Sql_cmd_tablespace /* purecov: inspected */
{
 public:
  /**
    Creates execution class instance for drop tablespace statement.

    @param tsname name of tablespace
    @param options additional options to statement
  */
  Sql_cmd_drop_tablespace(const LEX_STRING &tsname,
                          const Tablespace_options *options);
  bool execute(THD *) override;
};

/**
  Execution class for ALTER TABLESPACE ... tablespace_options
 */
class Sql_cmd_alter_tablespace final : public Sql_cmd_tablespace {
 public:
  /**
    Creates execution class instance for plain alter tablespace
    (modifying options).

    @param ts_name name of tablespace
    @param options additional options to statement
  */
  Sql_cmd_alter_tablespace(const LEX_STRING &ts_name,
                           const Tablespace_options *options);

  bool execute(THD *thd) override;
};

/**
  Execution class for ALTER TABLESPACE ... ADD DATAFILE ...
 */
class Sql_cmd_alter_tablespace_add_datafile final
    : public Sql_cmd_tablespace /* purecov: inspected */
{
  const LEX_STRING m_datafile_name;

 public:
  /**
    Creates execution class instance for add datafile statement.

    @param  tsname    name of tablespace
    @param  dfname    name of data file to add
    @param  options   additional options to statement
  */
  Sql_cmd_alter_tablespace_add_datafile(const LEX_STRING &tsname,
                                        const LEX_STRING &dfname,
                                        const Tablespace_options *options);
  bool execute(THD *) override;
};

/**
  Execution class for ALTER TABLESPACE ... DROP DATAFILE ...
 */
class Sql_cmd_alter_tablespace_drop_datafile final
    : public Sql_cmd_tablespace /* purecov: inspected */
{
  const LEX_STRING m_datafile_name;

 public:
  /**
    Creates execution class instance for drop datafile statement.

    @param  tsname    name of tablespace
    @param  dfname    name of data file to drop
    @param  options   additional options to statement
  */
  Sql_cmd_alter_tablespace_drop_datafile(const LEX_STRING &tsname,
                                         const LEX_STRING &dfname,
                                         const Tablespace_options *options);
  bool execute(THD *) override;
};

/**
  Execution class for ALTER TABLESPACE ... RENAME TO ...
 */
class Sql_cmd_alter_tablespace_rename final
    : public Sql_cmd_tablespace /* purecov: inspected */
{
  const LEX_STRING m_new_name;

 public:
  /**
    Creates execution class instance for rename statement.

    @param old_name existing tablespace
    @param new_name desired tablespace name
   */
  Sql_cmd_alter_tablespace_rename(const LEX_STRING &old_name,
                                  const LEX_STRING &new_name);
  bool execute(THD *) override;
};

/**
  Execution class for CREATE UNDO TABLESPACE
 */
class Sql_cmd_create_undo_tablespace final : public Sql_cmd {
  const ts_command_type m_cmd;
  const LEX_STRING m_undo_tablespace_name;
  const LEX_STRING m_datafile_name;
  const Tablespace_options *m_options;

 public:
  /**
    Creates execution class instance for undo tablespace statements.

    @param  cmd_type  subcommand passed to se
    @param  utsname   name of undo tablespace
    @param  dfname    name of data file
    @param  options   additional options to statemente
   */
  Sql_cmd_create_undo_tablespace(const ts_command_type cmd_type,
                                 const LEX_STRING &utsname,
                                 const LEX_STRING &dfname,
                                 const Tablespace_options *options);

  bool execute(THD *) override;
  enum_sql_command sql_command_code() const override;
};

/**
  Execution class for ALTER UNDO TABLESPACE
 */
class Sql_cmd_alter_undo_tablespace final : public Sql_cmd {
  const ts_command_type m_cmd;
  const LEX_STRING m_undo_tablespace_name;
  const LEX_STRING m_datafile_name;
  const ts_alter_tablespace_type m_at_type;
  const Tablespace_options *m_options;

 public:
  /**
    Creates execution class instance for undo tablespace statements.

    @param  cmd_type  subcommand passed to se
    @param  utsname   name of undo tablespace
    @param  dfname    name of data file
    @param  options   additional options to statemente
    @param  at_type   alter tablespace subcommand passed to se
   */
  Sql_cmd_alter_undo_tablespace(
      const ts_command_type cmd_type, const LEX_STRING &utsname,
      const LEX_STRING &dfname, const Tablespace_options *options,
      ts_alter_tablespace_type at_type = TS_ALTER_TABLESPACE_TYPE_NOT_DEFINED);

  bool execute(THD *) override;
  enum_sql_command sql_command_code() const override;
};

/**
  Execution class for DROP UNDO TABLESPACE
 */
class Sql_cmd_drop_undo_tablespace final : public Sql_cmd {
  const ts_command_type m_cmd;
  const LEX_STRING m_undo_tablespace_name;
  const LEX_STRING m_datafile_name;
  const Tablespace_options *m_options;

 public:
  /**
    Creates execution class instance for drop undo tablespace statements.

    @param  cmd_type  subcommand passed to se
    @param  utsname   name of undo tablespace
    @param  dfname    name of data file
    @param  options   additional options to statemente
   */
  Sql_cmd_drop_undo_tablespace(const ts_command_type cmd_type,
                               const LEX_STRING &utsname,
                               const LEX_STRING &dfname,
                               const Tablespace_options *options);

  bool execute(THD *) override;
  enum_sql_command sql_command_code() const override;
};

/**
  Execution class for CREATE/DROP/ALTER LOGFILE GROUP ...
 */
class Sql_cmd_logfile_group final : public Sql_cmd /* purecov: inspected */
{
  const ts_command_type m_cmd;
  const LEX_STRING m_logfile_group_name;
  const LEX_STRING m_undofile_name;
  const Tablespace_options *m_options;

 public:
  /**
    Creates execution class instance for logfile group statements.

    @param cmd_type subcommand passed to se
    @param logfile_group_name name of logfile group
    @param options additional options to statement
    @param undofile_name name of undo file
   */
  Sql_cmd_logfile_group(const ts_command_type cmd_type,
                        const LEX_STRING &logfile_group_name,
                        const Tablespace_options *options,
                        const LEX_STRING &undofile_name = {nullptr, 0});

  bool execute(THD *thd) override;

  enum_sql_command sql_command_code() const override;
};

#endif /* SQL_TABLESPACE_INCLUDED */
